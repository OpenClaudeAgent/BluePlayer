#include "FixtureLoader.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace blueplayer::test::e2e {

FixtureLoader::FixtureLoader(const QString& customPath)
    : m_fixturesPath(resolveFixturesPath(customPath))
    , m_valid(!m_fixturesPath.isEmpty())
{
    if (m_valid) {
        qInfo() << "[FixtureLoader] Fixtures path:" << m_fixturesPath;
    } else {
        qWarning() << "[FixtureLoader] Failed to find fixtures directory";
    }
}

bool FixtureLoader::isValid() const
{
    return m_valid;
}

QString FixtureLoader::fixturesPath() const
{
    return m_fixturesPath;
}

QString FixtureLoader::lastError() const
{
    return m_lastError;
}

std::optional<QJsonObject> FixtureLoader::loadJsonObject(const QString& filename)
{
    if (!m_valid) {
        m_lastError = "Fixtures directory not found";
        return std::nullopt;
    }

    QString path = fullPath(filename);
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Failed to open file: %1 (%2)").arg(path, file.errorString());
        qWarning() << "[FixtureLoader]" << m_lastError;
        return std::nullopt;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("JSON parse error in %1: %2 at offset %3")
                          .arg(filename, parseError.errorString())
                          .arg(parseError.offset);
        qWarning() << "[FixtureLoader]" << m_lastError;
        return std::nullopt;
    }

    if (!doc.isObject()) {
        m_lastError = QString("Expected JSON object in %1, got %2")
                          .arg(filename, doc.isArray() ? "array" : "other");
        qWarning() << "[FixtureLoader]" << m_lastError;
        return std::nullopt;
    }

    qDebug() << "[FixtureLoader] Loaded JSON object from" << filename;
    return doc.object();
}

std::optional<QJsonArray> FixtureLoader::loadJsonArray(const QString& filename)
{
    if (!m_valid) {
        m_lastError = "Fixtures directory not found";
        return std::nullopt;
    }

    QString path = fullPath(filename);
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Failed to open file: %1 (%2)").arg(path, file.errorString());
        qWarning() << "[FixtureLoader]" << m_lastError;
        return std::nullopt;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("JSON parse error in %1: %2 at offset %3")
                          .arg(filename, parseError.errorString())
                          .arg(parseError.offset);
        qWarning() << "[FixtureLoader]" << m_lastError;
        return std::nullopt;
    }

    // Handle both raw arrays and objects with "data" field
    QJsonArray result;
    if (doc.isArray()) {
        result = doc.array();
    } else if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("data") && obj["data"].isArray()) {
            result = obj["data"].toArray();
        } else {
            m_lastError = QString("Expected JSON array or object with 'data' field in %1").arg(filename);
            qWarning() << "[FixtureLoader]" << m_lastError;
            return std::nullopt;
        }
    } else {
        m_lastError = QString("Invalid JSON structure in %1").arg(filename);
        qWarning() << "[FixtureLoader]" << m_lastError;
        return std::nullopt;
    }

    qDebug() << "[FixtureLoader] Loaded JSON array from" << filename << "with" << result.size() << "items";
    return result;
}

std::optional<QByteArray> FixtureLoader::loadBinary(const QString& filename)
{
    if (!m_valid) {
        m_lastError = "Fixtures directory not found";
        return std::nullopt;
    }

    QString path = fullPath(filename);
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Failed to open file: %1 (%2)").arg(path, file.errorString());
        qWarning() << "[FixtureLoader]" << m_lastError;
        return std::nullopt;
    }

    QByteArray data = file.readAll();
    file.close();

    qDebug() << "[FixtureLoader] Loaded binary file" << filename << "(" << data.size() << "bytes)";
    return data;
}

bool FixtureLoader::exists(const QString& filename) const
{
    if (!m_valid) {
        return false;
    }
    return QFile::exists(fullPath(filename));
}

QString FixtureLoader::fullPath(const QString& filename) const
{
    return m_fixturesPath + "/" + filename;
}

QString FixtureLoader::resolveFixturesPath(const QString& customPath)
{
    // Priority 1: Custom path provided
    if (!customPath.isEmpty() && tryPath(customPath)) {
        return QDir(customPath).absolutePath();
    }

    // Priority 2: Environment variable
    QString envPath = QString::fromUtf8(qgetenv("E2E_FIXTURES_PATH"));
    if (!envPath.isEmpty() && tryPath(envPath)) {
        return QDir(envPath).absolutePath();
    }

#ifdef E2E_FIXTURES_PATH
    // Priority 3: Compile-time definition from CMake
    QString cmakePath = QStringLiteral(E2E_FIXTURES_PATH);
    if (tryPath(cmakePath)) {
        return QDir(cmakePath).absolutePath();
    }
#endif

    // Priority 4: Heuristic search
    QStringList candidates = {
        // From current working directory
        QDir::currentPath() + "/tests/e2e/fixtures",
        // From application directory (build tree)
        QCoreApplication::applicationDirPath() + "/../../../../../tests/e2e/fixtures",
        QCoreApplication::applicationDirPath() + "/../../fixtures",
        QCoreApplication::applicationDirPath() + "/../fixtures",
        // From source tree root
        QDir::currentPath() + "/../tests/e2e/fixtures",
    };

    for (const QString& candidate : candidates) {
        if (tryPath(candidate)) {
            return QDir(candidate).absolutePath();
        }
    }

    m_lastError = "Could not find fixtures directory. Set E2E_FIXTURES_PATH environment variable.";
    return QString();
}

bool FixtureLoader::tryPath(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return false;
    }

    // Verify it looks like a fixtures directory by checking for expected files
    QStringList expectedFiles = {"auth_token.json", "streams.json", "users.json"};
    for (const QString& file : expectedFiles) {
        if (!QFile::exists(dir.absoluteFilePath(file))) {
            return false;
        }
    }

    return true;
}

} // namespace blueplayer::test::e2e
