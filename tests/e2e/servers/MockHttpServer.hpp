#ifndef BLUEPLAYER_TEST_E2E_MOCKHTTPSERVER_HPP
#define BLUEPLAYER_TEST_E2E_MOCKHTTPSERVER_HPP

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>

namespace blueplayer::test::e2e {

/**
 * @brief Base class for HTTP mock servers.
 *
 * Provides common functionality for mock HTTP servers:
 * - TCP server lifecycle (start, stop, port management)
 * - HTTP request parsing
 * - HTTP response building
 * - Request logging and tracking
 *
 * Derived classes only need to implement handleRequest() for their specific endpoints.
 */
class MockHttpServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Parsed HTTP request structure.
     */
    struct HttpRequest {
        QString method;
        QString path;
        QString version;
        QMap<QString, QString> headers;
        QByteArray body;
        
        /**
         * @brief Get query string parameters.
         * @return Query string (everything after '?')
         */
        QString queryString() const;
        
        /**
         * @brief Get path without query string.
         */
        QString cleanPath() const;
        
        /**
         * @brief Get a query parameter value.
         * @param name Parameter name
         * @param defaultValue Value if not found
         */
        QString queryParam(const QString& name, const QString& defaultValue = QString()) const;
    };

    explicit MockHttpServer(QObject* parent = nullptr);
    ~MockHttpServer() override;

    // =========================================================================
    // Lifecycle
    // =========================================================================

    /**
     * @brief Start the server.
     * @param port Port to listen on (0 for auto-assign)
     * @return true if started successfully
     */
    bool start(quint16 port = 0);

    /**
     * @brief Stop the server.
     */
    void stop();

    /**
     * @brief Check if the server is running.
     */
    bool isRunning() const;

    /**
     * @brief Get the port the server is listening on.
     */
    quint16 port() const;

    /**
     * @brief Get the base URL of the server.
     * @return URL like "http://localhost:12345"
     */
    QString baseUrl() const;

    // =========================================================================
    // Request Tracking
    // =========================================================================

    /**
     * @brief Get the total number of requests received.
     */
    int requestCount() const;

    /**
     * @brief Get requests matching a path pattern.
     * @param pathPattern Substring to match in request paths
     */
    QStringList requestsTo(const QString& pathPattern) const;

    /**
     * @brief Clear the request log.
     */
    void clearRequests();

    /**
     * @brief Get all logged requests.
     */
    QStringList requestLog() const;

Q_SIGNALS:
    /**
     * @brief Emitted when a request is received.
     */
    void requestReceived(const QString& method, const QString& path);

protected:
    // =========================================================================
    // To be implemented by derived classes
    // =========================================================================

    /**
     * @brief Handle an HTTP request and return a response.
     *
     * Derived classes must implement this to handle their specific endpoints.
     *
     * @param request The parsed HTTP request
     * @return HTTP response bytes
     */
    virtual QByteArray handleRequest(const HttpRequest& request) = 0;

    /**
     * @brief Get the server name for logging.
     */
    virtual QString serverName() const = 0;

    // =========================================================================
    // Response Helpers (for derived classes)
    // =========================================================================

    /**
     * @brief Build an HTTP response.
     * @param statusCode HTTP status code
     * @param statusText HTTP status text
     * @param body Response body
     * @param contentType Content-Type header value
     */
    static QByteArray makeResponse(int statusCode, const QString& statusText,
                                   const QByteArray& body,
                                   const QString& contentType = "application/json");

    /**
     * @brief Build a JSON response with Twitch API format.
     * @param data The "data" array
     */
    static QByteArray makeTwitchResponse(const QJsonArray& data);

    /**
     * @brief Build a Twitch API error response.
     * @param statusCode HTTP status code
     * @param message Error message
     */
    static QByteArray makeTwitchError(int statusCode, const QString& message);

    /**
     * @brief Build a 404 Not Found response.
     */
    static QByteArray make404(const QString& message = "Not Found");

    // =========================================================================
    // Request Parsing (for derived classes that need custom parsing)
    // =========================================================================

    /**
     * @brief Parse raw HTTP request data.
     * @param data Raw request bytes
     */
    static HttpRequest parseRequest(const QByteArray& data);

private Q_SLOTS:
    void handleNewConnection();
    void handleClientData();
    void handleClientDisconnected();

private:
    QTcpServer* m_server = nullptr;
    quint16 m_port = 0;
    QStringList m_requestLog;
};

} // namespace blueplayer::test::e2e

#endif // BLUEPLAYER_TEST_E2E_MOCKHTTPSERVER_HPP
