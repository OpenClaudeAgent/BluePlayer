#ifndef BLUEPLAYER_TEST_E2E_FIXTURELOADER_HPP
#define BLUEPLAYER_TEST_E2E_FIXTURELOADER_HPP

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <optional>

namespace blueplayer::test::e2e {

/**
 * @brief Centralized fixture loader for E2E tests.
 *
 * Provides robust fixture path resolution and JSON/binary file loading
 * with clear error messages when files are missing or invalid.
 *
 * Usage:
 *   FixtureLoader loader;
 *   auto streams = loader.loadJsonArray("streams.json");
 *   auto segment = loader.loadBinary("test_segment.ts");
 */
class FixtureLoader
{
public:
    /**
     * @brief Construct a FixtureLoader.
     *
     * Resolves the fixtures path using the following priority:
     * 1. E2E_FIXTURES_PATH environment variable
     * 2. E2E_FIXTURES_PATH compile-time definition
     * 3. Heuristic search from current/app directory
     *
     * @param customPath Optional custom fixtures path (for testing)
     */
    explicit FixtureLoader(const QString& customPath = QString());

    /**
     * @brief Check if the fixtures directory was found.
     */
    bool isValid() const;

    /**
     * @brief Get the resolved fixtures path.
     */
    QString fixturesPath() const;

    /**
     * @brief Get the last error message.
     */
    QString lastError() const;

    /**
     * @brief Load a JSON file and return its content as a QJsonObject.
     * @param filename Name of the file (e.g., "auth_token.json")
     * @return The parsed JSON object, or std::nullopt on error
     */
    std::optional<QJsonObject> loadJsonObject(const QString& filename);

    /**
     * @brief Load a JSON file and return its content as a QJsonArray.
     *
     * Handles both raw arrays and objects with a "data" field containing an array.
     *
     * @param filename Name of the file (e.g., "streams.json")
     * @return The parsed JSON array, or std::nullopt on error
     */
    std::optional<QJsonArray> loadJsonArray(const QString& filename);

    /**
     * @brief Load a binary file (e.g., .ts video segment).
     * @param filename Name of the file (e.g., "test_segment.ts")
     * @return The file content, or std::nullopt on error
     */
    std::optional<QByteArray> loadBinary(const QString& filename);

    /**
     * @brief Check if a fixture file exists.
     * @param filename Name of the file
     */
    bool exists(const QString& filename) const;

    /**
     * @brief Get the full path to a fixture file.
     * @param filename Name of the file
     */
    QString fullPath(const QString& filename) const;

private:
    QString resolveFixturesPath(const QString& customPath);
    bool tryPath(const QString& path);

    QString m_fixturesPath;
    QString m_lastError;
    bool m_valid = false;
};

} // namespace blueplayer::test::e2e

#endif // BLUEPLAYER_TEST_E2E_FIXTURELOADER_HPP
