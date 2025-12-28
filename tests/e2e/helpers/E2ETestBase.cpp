#include "E2ETestBase.hpp"
#include "../servers/MockTwitchServer.hpp"
#include "../servers/MockHlsServer.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>

namespace blueplayer::test::e2e {

E2ETestBase::E2ETestBase(QObject* parent)
    : QObject(parent)
{
}

E2ETestBase::~E2ETestBase()
{
  delete m_twitchServer;
  delete m_hlsServer;
}

MockTwitchServer* E2ETestBase::twitchServer() const
{
  return m_twitchServer;
}

MockHlsServer* E2ETestBase::hlsServer() const
{
  return m_hlsServer;
}

QString E2ETestBase::twitchApiUrl() const
{
  return m_twitchServer ? m_twitchServer->baseUrl() : QString();
}

QString E2ETestBase::hlsServerUrl() const
{
  return m_hlsServer ? m_hlsServer->baseUrl() : QString();
}

void E2ETestBase::loadDefaultFixtures()
{
  loadStreamsFixture();
  loadUsersFixture();
  loadAuthFixture();
}

QJsonDocument E2ETestBase::loadFixture(const QString& filename) const
{
  QString path = fixturesPath() + "/" + filename;
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "E2ETestBase: Failed to open fixture file:" << path;
    return QJsonDocument();
  }

  QByteArray data = file.readAll();
  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(data, &error);

  if (error.error != QJsonParseError::NoError) {
    qWarning() << "E2ETestBase: Failed to parse fixture:" << path
               << "Error:" << error.errorString();
    return QJsonDocument();
  }

  return doc;
}

void E2ETestBase::loadStreamsFixture(const QString& filename)
{
  QJsonDocument doc = loadFixture(filename);
  if (doc.isNull()) return;

  QJsonObject root = doc.object();
  QJsonArray streams = root["data"].toArray();

  if (m_twitchServer) {
    m_twitchServer->setStreams(streams);

    // Also add channels to HLS server for each stream
    if (m_hlsServer) {
      for (const QJsonValue& stream : streams) {
        QString login = stream.toObject()["user_login"].toString();
        if (!login.isEmpty()) {
          m_hlsServer->addChannel(login);
        }
      }
    }
  }

  qDebug() << "E2ETestBase: Loaded" << streams.size() << "streams from" << filename;
}

void E2ETestBase::loadUsersFixture(const QString& filename)
{
  QJsonDocument doc = loadFixture(filename);
  if (doc.isNull()) return;

  QJsonObject root = doc.object();
  QJsonArray users = root["data"].toArray();

  if (m_twitchServer) {
    m_twitchServer->setUsers(users);
  }

  qDebug() << "E2ETestBase: Loaded" << users.size() << "users from" << filename;
}

void E2ETestBase::loadAuthFixture(const QString& filename)
{
  QJsonDocument doc = loadFixture(filename);
  if (doc.isNull()) return;

  QJsonObject root = doc.object();
  m_authToken = root["access_token"].toString();

  if (m_twitchServer && !m_authToken.isEmpty()) {
    m_twitchServer->setValidToken(m_authToken);
  }

  qDebug() << "E2ETestBase: Loaded auth token from" << filename;
}

void E2ETestBase::setupTestEnvironment(const QString& twitchApiUrl,
                                        const QString& hlsServerUrl,
                                        const QString& authToken)
{
  qputenv("BLUEPLAYER_TEST_MODE", "1");
  qputenv("BLUEPLAYER_MOCK_API_URL", twitchApiUrl.toUtf8());
  qputenv("BLUEPLAYER_MOCK_HLS_URL", hlsServerUrl.toUtf8());
  qputenv("BLUEPLAYER_MOCK_AUTH_TOKEN", authToken.toUtf8());
  qputenv("BLUEPLAYER_SKIP_AUTH", "1");

  qDebug() << "E2ETestBase: Test environment configured";
  qDebug() << "  API URL:" << twitchApiUrl;
  qDebug() << "  HLS URL:" << hlsServerUrl;
}

void E2ETestBase::clearTestEnvironment()
{
  qunsetenv("BLUEPLAYER_TEST_MODE");
  qunsetenv("BLUEPLAYER_MOCK_API_URL");
  qunsetenv("BLUEPLAYER_MOCK_HLS_URL");
  qunsetenv("BLUEPLAYER_MOCK_AUTH_TOKEN");
  qunsetenv("BLUEPLAYER_SKIP_AUTH");

  qDebug() << "E2ETestBase: Test environment cleared";
}

QString E2ETestBase::fixturesPath()
{
  // Try to find fixtures relative to the test executable or source
  QStringList searchPaths = {
    QCoreApplication::applicationDirPath() + "/../tests/e2e/fixtures",
    QCoreApplication::applicationDirPath() + "/fixtures",
    QCoreApplication::applicationDirPath() + "/../fixtures",
    "tests/e2e/fixtures",
    "../fixtures",
    "fixtures"
  };

#ifdef QUICK_TEST_SOURCE_DIR
  searchPaths.prepend(QString(QUICK_TEST_SOURCE_DIR) + "/../fixtures");
#endif

  for (const QString& path : searchPaths) {
    if (QDir(path).exists()) {
      return QDir(path).absolutePath();
    }
  }

  qWarning() << "E2ETestBase: Could not find fixtures directory";
  return QString();
}

void E2ETestBase::initTestCase()
{
  qDebug() << "E2ETestBase: Starting mock servers...";

  // Create and start mock servers
  m_twitchServer = new MockTwitchServer(this);
  m_hlsServer = new MockHlsServer(this);

  if (!m_twitchServer->start()) {
    qCritical() << "E2ETestBase: Failed to start MockTwitchServer";
    return;
  }

  if (!m_hlsServer->start()) {
    qCritical() << "E2ETestBase: Failed to start MockHlsServer";
    return;
  }

  // Configure HLS URL in Twitch server
  m_twitchServer->setHlsServerUrl(m_hlsServer->baseUrl());

  qDebug() << "E2ETestBase: Mock servers started";
  qDebug() << "  Twitch API:" << m_twitchServer->baseUrl();
  qDebug() << "  HLS Server:" << m_hlsServer->baseUrl();
}

void E2ETestBase::cleanupTestCase()
{
  qDebug() << "E2ETestBase: Stopping mock servers...";

  clearTestEnvironment();

  if (m_twitchServer) {
    m_twitchServer->stop();
  }

  if (m_hlsServer) {
    m_hlsServer->stop();
  }

  qDebug() << "E2ETestBase: Mock servers stopped";
}

void E2ETestBase::init()
{
  // Reset servers and load fixtures for each test
  if (m_twitchServer) {
    m_twitchServer->clearRequests();
    m_twitchServer->clearErrors();
  }

  if (m_hlsServer) {
    m_hlsServer->clearRequests();
    m_hlsServer->clearChannels();
  }

  // Load default fixtures
  loadDefaultFixtures();

  // Setup environment
  setupTestEnvironment(twitchApiUrl(), hlsServerUrl(), m_authToken);

  // Call subclass hook
  onSetUp();
}

void E2ETestBase::cleanup()
{
  // Call subclass hook
  onTearDown();
}

} // namespace blueplayer::test::e2e
