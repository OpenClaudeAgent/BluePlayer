#include "api/twitch/TwitchApiClient.hpp"

#include "core/Constants.hpp"
#include "core/Logger.hpp"
#include "core/InputValidator.hpp"

#include <QNetworkReply>
#include "core/NetworkCache.hpp"

using blueplayer::core::InputValidator;

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
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Access token set, length: %1").arg(token.length()));
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
  if (handleNetworkError(reply, QStringLiteral("récupération des streams"))) {
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Streams"));
  if (document.isNull()) {
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  const QVariantList streams = parseStreamsArray(entries);

  emit streamsReady(streams);
  reply->deleteLater();
}

void TwitchApiClient::getUserInfo() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getUserInfo() called"));
  if (m_accessToken.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("No access token"));
    emit errorOccurred(QStringLiteral("Aucun jeton d'accès Twitch."));
    return;
  }

  QUrl url(QStringLiteral("https://api.twitch.tv/helix/users"));
  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting user info from: %1").arg(url.toString()));
  QNetworkRequest request = buildRequest(url);
  QNetworkReply* reply = m_networkManager->get(request);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleUserInfoReply);
}

void TwitchApiClient::listFollowedStreams(const QString& userId, int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("listFollowedStreams() called with userId: %1, limit: %2").arg(userId).arg(limit));
  if (m_accessToken.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("No access token"));
    emit errorOccurred(QStringLiteral("Aucun jeton d'accès Twitch."));
    return;
  }

  // Validation d'entrée robuste
  if (userId.isEmpty() || !InputValidator::isValidTwitchUserId(userId)) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Invalid userId: %1").arg(userId));
    emit errorOccurred(QStringLiteral("ID utilisateur invalide."));
    return;
  }

  if (limit < 1 || limit > 100) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Invalid limit: %1").arg(limit));
    emit errorOccurred(QStringLiteral("Limite invalide (doit être entre 1 et 100)."));
    return;
  }

  QUrl url(QStringLiteral("https://api.twitch.tv/helix/streams/followed"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("user_id"), userId);
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting followed streams from: %1").arg(url.toString()));
  QNetworkRequest request = buildRequest(url);
  QNetworkReply* reply = m_networkManager->get(request);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleFollowedStreamsReply);
}

void TwitchApiClient::handleUserInfoReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleUserInfoReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des infos utilisateur"))) {
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("User info"));
  if (document.isNull()) {
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray data = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("User info data array size: %1").arg(data.size()));
  if (data.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("No user data found"));
    emit errorOccurred(QStringLiteral("Aucune donnée utilisateur trouvée."));
    reply->deleteLater();
    return;
  }

  const QJsonObject user = data.first().toObject();
  const QString userId = user.value(QStringLiteral("id")).toString();
  const QString userName = user.value(QStringLiteral("login")).toString();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("User ID: %1, Login: %2").arg(userId, userName));
  if (userId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Empty userId in response"));
    emit errorOccurred(QStringLiteral("ID utilisateur manquant dans la réponse."));
    reply->deleteLater();
    return;
  }

  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Emitting userInfoReady with userId: %1").arg(userId));
  emit userInfoReady(userId);
  reply->deleteLater();
}

void TwitchApiClient::handleFollowedStreamsReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleFollowedStreamsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des streams suivis"))) {
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Followed streams"));
  if (document.isNull()) {
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 followed streams").arg(entries.size()));
  const QVariantList streams = parseStreamsArray(entries);

  emit streamsReady(streams);
  reply->deleteLater();
}

bool TwitchApiClient::handleNetworkError(QNetworkReply* reply, const QString& errorContext) {
  if (!reply) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("No reply object in %1").arg(errorContext));
    return true;  // Erreur gérée
  }

  if (reply->error() != QNetworkReply::NoError) {
    const QByteArray payload = reply->readAll();
    core::Logger::error(core::LogCategory::Network, QStringLiteral("Network error in %1: %2").arg(errorContext, reply->errorString()));
    core::Logger::debug(core::LogCategory::Network, QStringLiteral("Response: %1").arg(QString::fromUtf8(payload)));
    emit errorOccurred(QStringLiteral("Erreur lors de %1: %2").arg(errorContext, reply->errorString()));
    reply->deleteLater();
    return true;  // Erreur gérée
  }

  return false;  // Pas d'erreur
}

QJsonDocument TwitchApiClient::parseJsonResponse(QNetworkReply* reply, const QString& errorContext) {
  const QByteArray payload = reply->readAll();
  core::Logger::debug(core::LogCategory::Network, QStringLiteral("%1 response status: %2").arg(errorContext).arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()));
  core::Logger::debug(core::LogCategory::Network, QStringLiteral("%1 response size: %2 bytes").arg(errorContext).arg(payload.size()));

  const QJsonDocument document = QJsonDocument::fromJson(payload);
  if (!document.isObject()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Invalid JSON response in %1").arg(errorContext));
    core::Logger::debug(core::LogCategory::Network, QStringLiteral("Response: %1").arg(QString::fromUtf8(payload)));
    emit errorOccurred(QStringLiteral("Réponse Twitch invalide pour %1.").arg(errorContext));
    reply->deleteLater();
    return QJsonDocument();
  }

  return document;
}

QVariantList TwitchApiClient::parseStreamsArray(const QJsonArray& entries) {
  QVariantList streams;
  streams.reserve(entries.size());

  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap stream;
    stream.insert(QStringLiteral("id"), entry.value(QStringLiteral("id")).toString());
    stream.insert(QStringLiteral("user_name"), entry.value(QStringLiteral("user_name")).toString());
    stream.insert(QStringLiteral("user_login"), entry.value(QStringLiteral("user_login")).toString());
    stream.insert(QStringLiteral("title"), entry.value(QStringLiteral("title")).toString());
    stream.insert(QStringLiteral("viewer_count"), entry.value(QStringLiteral("viewer_count")).toInt());
    stream.insert(QStringLiteral("language"), entry.value(QStringLiteral("language")).toString());
    stream.insert(QStringLiteral("thumbnail_url"), expandThumbnail(entry.value(QStringLiteral("thumbnail_url")).toString()));
    stream.insert(QStringLiteral("started_at"), entry.value(QStringLiteral("started_at")).toString());
    stream.insert(QStringLiteral("stream_url"), QStringLiteral("https://www.twitch.tv/%1").arg(entry.value(QStringLiteral("user_login")).toString()));
    streams.append(stream);
  }

  return streams;
}

QString TwitchApiClient::expandThumbnail(const QString& templateUrl) const {
  QString sanitized = templateUrl;
  sanitized.replace(QStringLiteral("{width}"), QString::number(core::constants::twitch::kThumbnailWidth));
  sanitized.replace(QStringLiteral("{height}"), QString::number(core::constants::twitch::kThumbnailHeight));
  return sanitized;
}

}  // namespace blueplayer::api::twitch

