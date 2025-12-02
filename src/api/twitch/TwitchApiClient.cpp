#include "api/twitch/TwitchApiClient.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

namespace blueplayer::api::twitch {

TwitchApiClient::TwitchApiClient(const QString& clientId, QObject* parent)
    : QObject(parent),
      m_clientId(clientId),
      m_networkManager(new QNetworkAccessManager(this)) {}

void TwitchApiClient::setAccessToken(const QString& token) {
  m_accessToken = token;
  qDebug() << "[TwitchApiClient] Access token set, length:" << token.length();
}

QNetworkRequest TwitchApiClient::buildRequest(const QUrl& url) const {
  QNetworkRequest request(url);
  request.setRawHeader("Client-Id", m_clientId.toUtf8());
  request.setRawHeader("Authorization",
                       QStringLiteral("Bearer %1").arg(m_accessToken).toUtf8());
  return request;
}

void TwitchApiClient::listStreams(int limit) {
  if (m_accessToken.isEmpty()) {
    emit errorOccurred(QStringLiteral("Aucun jeton d'accès Twitch."));
    return;
  }

  QUrl url(QStringLiteral("https://api.twitch.tv/helix/streams"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  QNetworkRequest request = buildRequest(url);
  QNetworkReply* reply = m_networkManager->get(request);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleReply);
}

void TwitchApiClient::handleReply() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    return;
  }

  const QByteArray payload = reply->readAll();
  if (reply->error() != QNetworkReply::NoError) {
    emit errorOccurred(reply->errorString());
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = QJsonDocument::fromJson(payload);
  if (!document.isObject()) {
    emit errorOccurred(QStringLiteral("Réponse Twitch invalide."));
    reply->deleteLater();
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  QVariantList streams;
  streams.reserve(entries.size());

  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap stream;
    stream.insert(QStringLiteral("id"), entry.value(QStringLiteral("id")).toString());
    stream.insert(QStringLiteral("user_name"),
                  entry.value(QStringLiteral("user_name")).toString());
    stream.insert(QStringLiteral("user_login"),
                  entry.value(QStringLiteral("user_login")).toString());
    stream.insert(QStringLiteral("title"), entry.value(QStringLiteral("title")).toString());
    stream.insert(QStringLiteral("viewer_count"),
                  entry.value(QStringLiteral("viewer_count")).toInt());
    stream.insert(QStringLiteral("language"),
                  entry.value(QStringLiteral("language")).toString());
    stream.insert(QStringLiteral("thumbnail_url"),
                  expandThumbnail(entry.value(QStringLiteral("thumbnail_url")).toString()));
    stream.insert(QStringLiteral("started_at"),
                  entry.value(QStringLiteral("started_at")).toString());
    stream.insert(QStringLiteral("stream_url"),
                  QStringLiteral("https://www.twitch.tv/%1")
                      .arg(entry.value(QStringLiteral("user_login")).toString()));
    streams.append(stream);
  }

  emit streamsReady(streams);
  reply->deleteLater();
}

void TwitchApiClient::getUserInfo() {
  qDebug() << "[TwitchApiClient] getUserInfo() called";
  if (m_accessToken.isEmpty()) {
    qDebug() << "[TwitchApiClient] ERROR: No access token";
    emit errorOccurred(QStringLiteral("Aucun jeton d'accès Twitch."));
    return;
  }

  QUrl url(QStringLiteral("https://api.twitch.tv/helix/users"));
  qDebug() << "[TwitchApiClient] Requesting user info from:" << url.toString();
  QNetworkRequest request = buildRequest(url);
  QNetworkReply* reply = m_networkManager->get(request);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleUserInfoReply);
}

void TwitchApiClient::listFollowedStreams(const QString& userId, int limit) {
  qDebug() << "[TwitchApiClient] listFollowedStreams() called with userId:" << userId << "limit:" << limit;
  if (m_accessToken.isEmpty()) {
    qDebug() << "[TwitchApiClient] ERROR: No access token";
    emit errorOccurred(QStringLiteral("Aucun jeton d'accès Twitch."));
    return;
  }

  if (userId.isEmpty()) {
    qDebug() << "[TwitchApiClient] ERROR: Empty userId";
    emit errorOccurred(QStringLiteral("ID utilisateur manquant."));
    return;
  }

  QUrl url(QStringLiteral("https://api.twitch.tv/helix/streams/followed"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("user_id"), userId);
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  qDebug() << "[TwitchApiClient] Requesting followed streams from:" << url.toString();
  QNetworkRequest request = buildRequest(url);
  QNetworkReply* reply = m_networkManager->get(request);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleFollowedStreamsReply);
}

void TwitchApiClient::handleUserInfoReply() {
  qDebug() << "[TwitchApiClient] handleUserInfoReply() called";
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    qDebug() << "[TwitchApiClient] ERROR: No reply object";
    return;
  }

  const QByteArray payload = reply->readAll();
  qDebug() << "[TwitchApiClient] User info response status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  qDebug() << "[TwitchApiClient] User info response size:" << payload.size() << "bytes";
  
  if (reply->error() != QNetworkReply::NoError) {
    qDebug() << "[TwitchApiClient] ERROR:" << reply->errorString();
    qDebug() << "[TwitchApiClient] Response:" << QString::fromUtf8(payload);
    emit errorOccurred(QStringLiteral("Erreur lors de la récupération des infos utilisateur: %1")
                           .arg(reply->errorString()));
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = QJsonDocument::fromJson(payload);
  if (!document.isObject()) {
    emit errorOccurred(QStringLiteral("Réponse Twitch invalide pour les infos utilisateur."));
    reply->deleteLater();
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray data = root.value(QStringLiteral("data")).toArray();
  qDebug() << "[TwitchApiClient] User info data array size:" << data.size();
  if (data.isEmpty()) {
    qDebug() << "[TwitchApiClient] ERROR: No user data found";
    qDebug() << "[TwitchApiClient] Response JSON:" << QString::fromUtf8(payload);
    emit errorOccurred(QStringLiteral("Aucune donnée utilisateur trouvée."));
    reply->deleteLater();
    return;
  }

  const QJsonObject user = data.first().toObject();
  const QString userId = user.value(QStringLiteral("id")).toString();
  const QString userName = user.value(QStringLiteral("login")).toString();
  qDebug() << "[TwitchApiClient] User ID:" << userId << "Login:" << userName;
  if (userId.isEmpty()) {
    qDebug() << "[TwitchApiClient] ERROR: Empty userId in response";
    emit errorOccurred(QStringLiteral("ID utilisateur manquant dans la réponse."));
    reply->deleteLater();
    return;
  }

  qDebug() << "[TwitchApiClient] Emitting userInfoReady with userId:" << userId;
  emit userInfoReady(userId);
  reply->deleteLater();
}

void TwitchApiClient::handleFollowedStreamsReply() {
  qDebug() << "[TwitchApiClient] handleFollowedStreamsReply() called";
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    qDebug() << "[TwitchApiClient] ERROR: No reply object";
    return;
  }

  const QByteArray payload = reply->readAll();
  qDebug() << "[TwitchApiClient] Followed streams response status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  qDebug() << "[TwitchApiClient] Followed streams response size:" << payload.size() << "bytes";
  
  if (reply->error() != QNetworkReply::NoError) {
    qDebug() << "[TwitchApiClient] ERROR:" << reply->errorString();
    qDebug() << "[TwitchApiClient] Response:" << QString::fromUtf8(payload);
    emit errorOccurred(QStringLiteral("Erreur lors de la récupération des streams suivis: %1")
                           .arg(reply->errorString()));
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = QJsonDocument::fromJson(payload);
  if (!document.isObject()) {
    qDebug() << "[TwitchApiClient] ERROR: Invalid JSON response";
    qDebug() << "[TwitchApiClient] Response:" << QString::fromUtf8(payload);
    emit errorOccurred(QStringLiteral("Réponse Twitch invalide pour les streams suivis."));
    reply->deleteLater();
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  qDebug() << "[TwitchApiClient] Found" << entries.size() << "followed streams";
  QVariantList streams;
  streams.reserve(entries.size());

  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap stream;
    stream.insert(QStringLiteral("id"), entry.value(QStringLiteral("id")).toString());
    stream.insert(QStringLiteral("user_name"),
                  entry.value(QStringLiteral("user_name")).toString());
    stream.insert(QStringLiteral("user_login"),
                  entry.value(QStringLiteral("user_login")).toString());
    stream.insert(QStringLiteral("title"), entry.value(QStringLiteral("title")).toString());
    stream.insert(QStringLiteral("viewer_count"),
                  entry.value(QStringLiteral("viewer_count")).toInt());
    stream.insert(QStringLiteral("language"),
                  entry.value(QStringLiteral("language")).toString());
    stream.insert(QStringLiteral("thumbnail_url"),
                  expandThumbnail(entry.value(QStringLiteral("thumbnail_url")).toString()));
    stream.insert(QStringLiteral("started_at"),
                  entry.value(QStringLiteral("started_at")).toString());
    stream.insert(QStringLiteral("stream_url"),
                  QStringLiteral("https://www.twitch.tv/%1")
                      .arg(entry.value(QStringLiteral("user_login")).toString()));
    streams.append(stream);
  }

  emit streamsReady(streams);
  reply->deleteLater();
}

QString TwitchApiClient::expandThumbnail(const QString& templateUrl) const {
  QString sanitized = templateUrl;
  sanitized.replace(QStringLiteral("{width}"), QStringLiteral("320"));
  sanitized.replace(QStringLiteral("{height}"), QStringLiteral("180"));
  return sanitized;
}

}  // namespace blueplayer::api::twitch

