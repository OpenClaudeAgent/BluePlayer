#pragma once

#include <QObject>
#include <QVariantList>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QUrl;
QT_END_NAMESPACE

namespace blueplayer::api::twitch {

class TwitchApiClient : public QObject {
  Q_OBJECT

public:
  explicit TwitchApiClient(const QString& clientId, QObject* parent = nullptr);

  void setAccessToken(const QString& token);
  Q_INVOKABLE void listStreams(int limit = 12);  // Utilise constants::twitch::kDefaultStreamListLimit
  Q_INVOKABLE void getUserInfo();
  Q_INVOKABLE void listFollowedStreams(const QString& userId, int limit = 100);  // Utilise constants::twitch::kDefaultStreamLimit

signals:
  void streamsReady(const QVariantList& streams);
  void userInfoReady(const QString& userId);
  void errorOccurred(const QString& message);

private slots:
  void handleReply();
  void handleUserInfoReply();
  void handleFollowedStreamsReply();

private:
  QString expandThumbnail(const QString& templateUrl) const;
  QNetworkRequest buildRequest(const QUrl& url) const;
  
  // Méthodes génériques pour réduire la duplication
  bool handleNetworkError(QNetworkReply* reply, const QString& errorContext);
  QJsonDocument parseJsonResponse(QNetworkReply* reply, const QString& errorContext);
  QVariantList parseStreamsArray(const QJsonArray& entries);
  
  QString m_clientId;
  QString m_accessToken;
  QNetworkAccessManager* m_networkManager = nullptr;
};

}  // namespace blueplayer::api::twitch

