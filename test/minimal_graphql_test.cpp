// Minimal GraphQL test to isolate if issue is query-specific or general
// This can be compiled and run to test different GraphQL queries

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // Get Client-ID and token from environment
    QString clientId = QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID"));
    QString token = QString::fromUtf8(qgetenv("TWITCH_ACCESS_TOKEN"));
    
    if (clientId.isEmpty() || token.isEmpty()) {
        qDebug() << "ERROR: TWITCH_CLIENT_ID and TWITCH_ACCESS_TOKEN must be set";
        return 1;
    }
    
    qDebug() << "Testing minimal GraphQL query...";
    qDebug() << "Client-ID:" << clientId;
    qDebug() << "Token length:" << token.length();
    
    // Minimal GraphQL query - just get current user info
    QJsonObject queryObject;
    queryObject["query"] = "{ currentUser { id login displayName } }";
    
    QJsonDocument doc(queryObject);
    QUrl url("https://gql.twitch.tv/gql");
    
    QNetworkRequest request(url);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Client-ID", clientId.toUtf8());  // Try exact case
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    
    qDebug() << "Request URL:" << url.toString();
    qDebug() << "Headers set:";
    qDebug() << "  Content-Type: application/json";
    qDebug() << "  Client-ID:" << clientId;
    qDebug() << "  Authorization: Bearer ***";
    
    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.post(request, doc.toJson(QJsonDocument::Compact));
    
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);  // 10 second timeout
    
    loop.exec();
    
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "ERROR:" << reply->errorString();
        qDebug() << "Response:" << reply->readAll();
        return 1;
    }
    
    QByteArray response = reply->readAll();
    qDebug() << "SUCCESS! Response:" << response;
    
    QJsonDocument responseDoc = QJsonDocument::fromJson(response);
    if (!responseDoc.isNull()) {
        qDebug() << "Parsed JSON:" << responseDoc.toJson(QJsonDocument::Indented);
    }
    
    reply->deleteLater();
    return 0;
}


