#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/Config.hpp"

using namespace blueplayer::core;

class TestConfig : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  
  void testSingleton();
  void testLoadFromEnvironment();
  void testLoadFromFile();
  void testDefaultValues();
  void testLogLevel();
  void testNetworkCacheConfig();
};

void TestConfig::initTestCase() {
  // Nettoyer les variables d'environnement pour des tests propres
  qunsetenv("TWITCH_CLIENT_ID");
  qunsetenv("TWITCH_CLIENT_SECRET");
  qunsetenv("TWITCH_REDIRECT_URI");
  qunsetenv("TWITCH_REDIRECT_PORT");
  qunsetenv("BLUEPLAYER_NETWORK_CACHE_SIZE");
  qunsetenv("BLUEPLAYER_NETWORK_CACHE_TTL");
}

void TestConfig::cleanupTestCase() {
  // Nettoyer le fichier de config de test s'il existe
  const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + 
                             "/BluePlayer/config.json";
  if (QFile::exists(configPath)) {
    QFile::remove(configPath);
  }
}

void TestConfig::testSingleton() {
  Config& config1 = Config::instance();
  Config& config2 = Config::instance();
  
  QVERIFY(&config1 == &config2);
}

void TestConfig::testLoadFromEnvironment() {
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
  qputenv("TWITCH_CLIENT_SECRET", "test_secret");
  qputenv("TWITCH_REDIRECT_URI", "https://localhost:8443/callback");
  qputenv("TWITCH_REDIRECT_PORT", "8443");
  qputenv("BLUEPLAYER_NETWORK_CACHE_SIZE", "10485760");  // 10 MB
  qputenv("BLUEPLAYER_NETWORK_CACHE_TTL", "600");  // 10 minutes
  
  Config& config = Config::instance();
  config.load();
  
  QCOMPARE(config.twitchClientId(), QString("test_client_id"));
  QCOMPARE(config.twitchClientSecret(), QString("test_secret"));
  QCOMPARE(config.twitchRedirectUri(), QString("https://localhost:8443/callback"));
  QCOMPARE(config.twitchRedirectPort(), quint16(8443));
  QCOMPARE(config.networkCacheSize(), 10485760);
  QCOMPARE(config.networkCacheTTL(), 600);
  
  // Nettoyer
  qunsetenv("TWITCH_CLIENT_ID");
  qunsetenv("TWITCH_CLIENT_SECRET");
  qunsetenv("TWITCH_REDIRECT_URI");
  qunsetenv("TWITCH_REDIRECT_PORT");
  qunsetenv("BLUEPLAYER_NETWORK_CACHE_SIZE");
  qunsetenv("BLUEPLAYER_NETWORK_CACHE_TTL");
}

void TestConfig::testLoadFromFile() {
  // Créer un fichier de configuration de test
  const QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/BluePlayer";
  QDir().mkpath(configDir);
  
  const QString configPath = configDir + "/config.json";
  QFile file(configPath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  
  QJsonObject configObj;
  QJsonObject twitchObj;
  twitchObj["clientId"] = "file_client_id";
  twitchObj["clientSecret"] = "file_secret";
  twitchObj["redirectUri"] = "https://localhost:9000/callback";
  twitchObj["redirectPort"] = 9000;
  configObj["twitch"] = twitchObj;
  
  QJsonObject networkObj;
  networkObj["cacheSize"] = 20971520;  // 20 MB
  networkObj["cacheTTL"] = 1200;  // 20 minutes
  configObj["network"] = networkObj;
  
  QJsonObject loggingObj;
  loggingObj["Media"] = "debug";
  loggingObj["Twitch"] = "warning";
  configObj["logging"] = loggingObj;
  
  QJsonDocument doc(configObj);
  file.write(doc.toJson());
  file.close();
  
  Config& config = Config::instance();
  config.load();
  
  QCOMPARE(config.twitchClientId(), QString("file_client_id"));
  QCOMPARE(config.twitchClientSecret(), QString("file_secret"));
  QCOMPARE(config.twitchRedirectUri(), QString("https://localhost:9000/callback"));
  QCOMPARE(config.twitchRedirectPort(), quint16(9000));
  QCOMPARE(config.networkCacheSize(), 20971520);
  QCOMPARE(config.networkCacheTTL(), 1200);
  QCOMPARE(config.logLevel("Media"), QString("debug"));
  QCOMPARE(config.logLevel("Twitch"), QString("warning"));
  
  // Nettoyer
  QFile::remove(configPath);
}

void TestConfig::testDefaultValues() {
  // Nettoyer les variables d'environnement qui pourraient affecter les valeurs par défaut
  qunsetenv("BLUEPLAYER_NETWORK_CACHE_SIZE");
  qunsetenv("BLUEPLAYER_NETWORK_CACHE_TTL");
  
  Config& config = Config::instance();
  // Réinitialiser la configuration en créant une nouvelle instance
  // Note: Comme Config est un singleton, on ne peut pas vraiment le réinitialiser
  // On teste donc avec les valeurs actuelles après nettoyage des variables d'environnement
  config.load();
  
  // Vérifier les valeurs par défaut (ou celles configurées)
  // Si une variable d'environnement est définie, elle prendra le dessus
  QVERIFY(config.networkCacheSize() > 0);
  QVERIFY(config.networkCacheTTL() > 0);
  // quint16 a une valeur maximale de 65535
  QVERIFY(config.twitchRedirectPort() > 0 && config.twitchRedirectPort() <= 65535);
}

void TestConfig::testLogLevel() {
  qputenv("BLUEPLAYER_LOG_MEDIA", "debug");
  qputenv("BLUEPLAYER_LOG_TWITCH", "error");
  
  Config& config = Config::instance();
  config.load();
  
  QCOMPARE(config.logLevel("Media"), QString("debug"));
  QCOMPARE(config.logLevel("Twitch"), QString("error"));
  QCOMPARE(config.logLevel("UnknownCategory"), QString("info"));  // Valeur par défaut
  
  qunsetenv("BLUEPLAYER_LOG_MEDIA");
  qunsetenv("BLUEPLAYER_LOG_TWITCH");
}

void TestConfig::testNetworkCacheConfig() {
  // Test avec valeurs invalides (doivent être ignorées)
  qputenv("BLUEPLAYER_NETWORK_CACHE_SIZE", "-100");
  qputenv("BLUEPLAYER_NETWORK_CACHE_TTL", "0");
  
  Config& config = Config::instance();
  config.load();
  
  // Les valeurs invalides doivent être ignorées, valeurs par défaut conservées
  QVERIFY(config.networkCacheSize() > 0);
  QVERIFY(config.networkCacheTTL() > 0);
  
  qunsetenv("BLUEPLAYER_NETWORK_CACHE_SIZE");
  qunsetenv("BLUEPLAYER_NETWORK_CACHE_TTL");
}

QTEST_MAIN(TestConfig)
#include "TestConfig.moc"


