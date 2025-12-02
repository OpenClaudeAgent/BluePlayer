#include "api/twitch/TwitchApiClient.hpp"

#include "core/Constants.hpp"
#include "core/Logger.hpp"
#include "core/InputValidator.hpp"
#include "core/ErrorHandler.hpp"

using blueplayer::core::InputValidator;
using blueplayer::core::ErrorHandler;

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>

namespace blueplayer::api::twitch {

TwitchApiClient::TwitchApiClient(const QString& clientId, QObject* parent)
    : ApiClientBase(parent),
      m_clientId(clientId) {
  // Configurer le header Client-Id par défaut
  setDefaultHeader(QStringLiteral("Client-Id"), clientId);
  
  // Connecter les erreurs de ApiClientBase vers le signal QString pour compatibilité QML
  connect(this, &ApiClientBase::errorOccurred, this, [this](const blueplayer::core::Error& error) {
    emit errorOccurred(error.toString());
  });
}

void TwitchApiClient::setAccessToken(const QString& token) {
  setBearerToken(token);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Access token set, length: %1").arg(token.length()));
}

void TwitchApiClient::listStreams(int limit) {
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/streams"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleReply);
}

void TwitchApiClient::handleReply() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des streams"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Streams"));
  reply->deleteLater();
  if (document.isNull()) {
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  const QVariantList streams = parseStreamsArray(entries);

  emit streamsReady(streams);
}

void TwitchApiClient::getUserInfo() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getUserInfo() called"));
  
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/users"));
  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting user info from: %1").arg(url.toString()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleUserInfoReply);
}

void TwitchApiClient::listFollowedStreams(const QString& userId, int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("listFollowedStreams() called with userId: %1, limit: %2").arg(userId).arg(limit));

  // Validation d'entrée robuste
  if (userId.isEmpty() || !InputValidator::isValidTwitchUserId(userId)) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Invalid userId: %1").arg(userId));
    const auto error = ErrorHandler::validationError(QStringLiteral("listFollowedStreams"), QStringLiteral("ID utilisateur invalide"));
    emit errorOccurred(error.toString());
    return;
  }

  if (limit < 1 || limit > 100) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Invalid limit: %1").arg(limit));
    const auto error = ErrorHandler::validationError(QStringLiteral("listFollowedStreams"), QStringLiteral("Limite invalide (doit être entre 1 et 100)"));
    emit errorOccurred(error.toString());
    return;
  }

  QUrl url(QStringLiteral("https://api.twitch.tv/helix/streams/followed"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("user_id"), userId);
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting followed streams from: %1").arg(url.toString()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleFollowedStreamsReply);
}

void TwitchApiClient::handleUserInfoReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleUserInfoReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des infos utilisateur"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("User info"));
  reply->deleteLater();
  if (document.isNull()) {
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray data = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("User info data array size: %1").arg(data.size()));
  if (data.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("No user data found"));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getUserInfo"), QStringLiteral("Aucune donnée utilisateur trouvée"));
    emit errorOccurred(error.toString());
    return;
  }

  const QJsonObject user = data.first().toObject();
  const QString userId = user.value(QStringLiteral("id")).toString();
  const QString userName = user.value(QStringLiteral("login")).toString();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("User ID: %1, Login: %2").arg(userId, userName));
  if (userId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Empty userId in response"));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getUserInfo"), QStringLiteral("ID utilisateur manquant dans la réponse"));
    emit errorOccurred(error.toString());
    return;
  }

  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Emitting userInfoReady with userId: %1").arg(userId));
  emit userInfoReady(userId);
}

void TwitchApiClient::handleFollowedStreamsReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleFollowedStreamsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des streams suivis"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Followed streams"));
  reply->deleteLater();
  if (document.isNull()) {
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 followed streams").arg(entries.size()));
  const QVariantList streams = parseStreamsArray(entries);

  emit streamsReady(streams);
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

