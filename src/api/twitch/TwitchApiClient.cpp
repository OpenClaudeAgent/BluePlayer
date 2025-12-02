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

void TwitchApiClient::getRecommendedStreams(int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getRecommendedStreams() called with limit: %1").arg(limit));
  
  // Vérifier que le Client-ID est configuré
  if (m_clientId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Client ID is empty! Cannot request recommended streams."));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getRecommendedStreams"), QStringLiteral("Client ID manquant"));
    emit errorOccurred(error.toString());
    return;
  }
  
  // Utiliser l'endpoint /streams pour obtenir les streams populaires comme recommandations
  // On peut filtrer par game_id ou language si nécessaire pour personnaliser
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/streams"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  // Optionnel: filtrer par langue ou jeu pour personnaliser les recommandations
  // query.addQueryItem(QStringLiteral("language"), QStringLiteral("fr"));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting recommended streams from: %1").arg(url.toString()));
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Client ID configured: %1").arg(m_clientId.isEmpty() ? QStringLiteral("EMPTY") : m_clientId.left(10) + "..."));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleRecommendedStreamsReply);
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

void TwitchApiClient::handleRecommendedStreamsReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleRecommendedStreamsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des streams recommandés"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Recommended streams"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("Failed to parse recommended streams response"));
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 recommended streams").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No recommended streams found in API response"));
    // Émettre une liste vide pour indiquer qu'il n'y a pas de données
    emit recommendedStreamsReady(QVariantList());
    return;
  }
  
  const QVariantList streams = parseStreamsArray(entries);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Parsed %1 recommended streams, emitting signal").arg(streams.size()));
  emit recommendedStreamsReady(streams);
}

void TwitchApiClient::getTopCategories(int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getTopCategories() called with limit: %1").arg(limit));
  
  // Vérifier que le Client-ID est configuré
  if (m_clientId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Client ID is empty! Cannot request top categories."));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getTopCategories"), QStringLiteral("Client ID manquant"));
    emit errorOccurred(error.toString());
    return;
  }
  
  // Utiliser l'endpoint /games/top pour obtenir les catégories/jeux populaires
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/games/top"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting top categories from: %1").arg(url.toString()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleCategoriesReply);
}

void TwitchApiClient::getPopularClips(int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getPopularClips() called with limit: %1").arg(limit));
  
  if (m_clientId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Client ID is empty! Cannot request popular clips."));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getPopularClips"), QStringLiteral("Client ID manquant"));
    emit errorOccurred(error.toString());
    return;
  }
  
  // L'API Twitch nécessite un game_id ou broadcaster_id pour /clips
  // Utiliser "Just Chatting" (game_id: 509658) comme catégorie populaire par défaut
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/clips"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("game_id"), QStringLiteral("509658"));  // Just Chatting
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting popular clips from: %1").arg(url.toString()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handlePopularClipsReply);
}

void TwitchApiClient::getFollowedClips(const QStringList& broadcasterIds, int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] getFollowedClips() called with %1 broadcasters, limit: %2").arg(broadcasterIds.size()).arg(limit));
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Broadcaster IDs: %1").arg(broadcasterIds.join(", ")));
  
  if (m_clientId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Client ID is empty! Cannot request followed clips."));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getFollowedClips"), QStringLiteral("Client ID manquant"));
    emit errorOccurred(error.toString());
    return;
  }
  
  if (broadcasterIds.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("[DEBUG] No broadcaster IDs provided, emitting empty list"));
    emit followedClipsReady(QVariantList());
    return;
  }
  
  // Utiliser l'endpoint /clips avec broadcaster_id pour obtenir les clips des streamers suivis
  // Note: L'API Twitch peut accepter plusieurs broadcaster_id séparés par des virgules,
  // mais pour éviter les problèmes, on limite à 10 broadcaster_id maximum par requête
  // et on utilise seulement les premiers pour cette requête
  const int maxBroadcastersPerRequest = 10;
  QStringList limitedBroadcasterIds = broadcasterIds.mid(0, maxBroadcastersPerRequest);
  
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/clips"));
  QUrlQuery query;
  QString broadcasterIdsStr = limitedBroadcasterIds.join(QStringLiteral(","));
  query.addQueryItem(QStringLiteral("broadcaster_id"), broadcasterIdsStr);
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("[DEBUG] Requesting followed clips from: %1").arg(url.toString()));
  core::Logger::debug(core::LogCategory::Network, QStringLiteral("[DEBUG] Using %1/%2 broadcaster IDs: %3").arg(limitedBroadcasterIds.size()).arg(broadcasterIds.size()).arg(broadcasterIdsStr));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleFollowedClipsReply);
}

void TwitchApiClient::getVideos(const QString& userId, int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getVideos() called for userId: %1, limit: %2").arg(userId).arg(limit));
  
  if (m_clientId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Client ID is empty! Cannot request videos."));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getVideos"), QStringLiteral("Client ID manquant"));
    emit errorOccurred(error.toString());
    return;
  }
  
  if (userId.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No userId provided, emitting empty list"));
    emit videosReady(QVariantList());
    return;
  }
  
  // Utiliser l'endpoint /videos pour obtenir les VODs archivés
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/videos"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("user_id"), userId);
  query.addQueryItem(QStringLiteral("type"), QStringLiteral("archive"));  // Seulement les VODs archivés
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting videos from: %1").arg(url.toString()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleVideosReply);
}

void TwitchApiClient::handleCategoriesReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleCategoriesReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des catégories"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Top categories"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("Failed to parse categories response"));
    // Émettre une liste vide pour indiquer qu'il n'y a pas de données (comme handleRecommendedStreamsReply)
    emit categoriesReady(QVariantList());
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 categories").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No categories found in API response"));
    // Émettre une liste vide pour indiquer qu'il n'y a pas de données
    emit categoriesReady(QVariantList());
    return;
  }
  
  QVariantList categories;
  categories.reserve(entries.size());
  
  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap category;
    
    // Récupérer les informations de la catégorie
    category.insert(QStringLiteral("id"), entry.value(QStringLiteral("id")).toString());
    category.insert(QStringLiteral("name"), entry.value(QStringLiteral("name")).toString());
    category.insert(QStringLiteral("boxArtUrl"), entry.value(QStringLiteral("box_art_url")).toString());
    
    // Le box_art_url contient des placeholders {width} et {height}, les remplacer
    QString boxArtUrl = entry.value(QStringLiteral("box_art_url")).toString();
    boxArtUrl.replace(QStringLiteral("{width}"), QString::number(285));  // Taille standard pour les catégories
    boxArtUrl.replace(QStringLiteral("{height}"), QString::number(380));
    category.insert(QStringLiteral("boxArtUrl"), boxArtUrl);
    
    categories.append(category);
  }
  
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Parsed %1 categories, emitting signal").arg(categories.size()));
  emit categoriesReady(categories);
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
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] handleFollowedStreamsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des streams suivis"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Followed streams"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Failed to parse followed streams response"));
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Found %1 followed streams in API response").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("[DEBUG] No followed streams found - user may not be following any live channels"));
    emit streamsReady(QVariantList());
    return;
  }
  
  const QVariantList streams = parseStreamsArray(entries);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Parsed %1 followed streams, emitting streamsReady signal").arg(streams.size()));
  
  // Log les broadcaster_ids extraits pour debug
  for (int i = 0; i < qMin(streams.size(), 5); ++i) {
    const QVariantMap stream = streams.at(i).toMap();
    core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Stream %1: user_id=%2, user_name=%3").arg(i).arg(stream.value(QStringLiteral("user_id")).toString()).arg(stream.value(QStringLiteral("user_name")).toString()));
  }

  emit streamsReady(streams);
}


QVariantList TwitchApiClient::parseStreamsArray(const QJsonArray& entries) {
  QVariantList streams;
  streams.reserve(entries.size());

  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap stream;
    stream.insert(QStringLiteral("id"), entry.value(QStringLiteral("id")).toString());
    stream.insert(QStringLiteral("user_id"), entry.value(QStringLiteral("user_id")).toString());  // broadcaster_id pour les clips
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

void TwitchApiClient::handlePopularClipsReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handlePopularClipsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des clips populaires"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Popular clips"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("Failed to parse popular clips response"));
    emit popularClipsReady(QVariantList());
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 popular clips").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No popular clips found in API response"));
    emit popularClipsReady(QVariantList());
    return;
  }
  
  const QVariantList clips = parseClipsArray(entries);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Parsed %1 popular clips, emitting signal").arg(clips.size()));
  emit popularClipsReady(clips);
}

void TwitchApiClient::handleFollowedClipsReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] handleFollowedClipsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  
  if (!reply) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("[DEBUG] No reply object"));
    emit followedClipsReady(QVariantList());
    return;
  }
  
  int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] HTTP status: %1").arg(httpStatus));
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Response URL: %1").arg(reply->url().toString()));
  
  if (handleNetworkError(reply, QStringLiteral("récupération des clips suivis"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Followed clips"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Failed to parse followed clips response (document is null)"));
    emit followedClipsReady(QVariantList());
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Found %1 followed clips in API response").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("[DEBUG] No followed clips found in API response (empty data array)"));
    emit followedClipsReady(QVariantList());
    return;
  }
  
  const QVariantList clips = parseClipsArray(entries);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Parsed %1 followed clips, emitting signal").arg(clips.size()));
  emit followedClipsReady(clips);
}

QVariantList TwitchApiClient::parseClipsArray(const QJsonArray& entries) {
  QVariantList clips;
  clips.reserve(entries.size());

  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap clip;
    
    clip.insert(QStringLiteral("id"), entry.value(QStringLiteral("id")).toString());
    clip.insert(QStringLiteral("title"), entry.value(QStringLiteral("title")).toString());
    clip.insert(QStringLiteral("broadcaster_name"), entry.value(QStringLiteral("broadcaster_name")).toString());
    clip.insert(QStringLiteral("broadcaster_id"), entry.value(QStringLiteral("broadcaster_id")).toString());
    clip.insert(QStringLiteral("view_count"), entry.value(QStringLiteral("view_count")).toInt());
    clip.insert(QStringLiteral("duration"), entry.value(QStringLiteral("duration")).toDouble());
    clip.insert(QStringLiteral("thumbnail_url"), entry.value(QStringLiteral("thumbnail_url")).toString());
    clip.insert(QStringLiteral("created_at"), entry.value(QStringLiteral("created_at")).toString());
    clip.insert(QStringLiteral("url"), entry.value(QStringLiteral("url")).toString());
    
    clips.append(clip);
  }

  return clips;
}

void TwitchApiClient::handleVideosReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleVideosReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des VODs"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Videos"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("Failed to parse videos response"));
    emit videosReady(QVariantList());
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 videos").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No videos found in API response"));
    emit videosReady(QVariantList());
    return;
  }
  
  const QVariantList videos = parseVideosArray(entries);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Parsed %1 videos, emitting signal").arg(videos.size()));
  emit videosReady(videos);
}

QVariantList TwitchApiClient::parseVideosArray(const QJsonArray& entries) {
  QVariantList videos;
  videos.reserve(entries.size());

  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap video;
    
    video.insert(QStringLiteral("id"), entry.value(QStringLiteral("id")).toString());
    video.insert(QStringLiteral("title"), entry.value(QStringLiteral("title")).toString());
    video.insert(QStringLiteral("user_name"), entry.value(QStringLiteral("user_name")).toString());
    video.insert(QStringLiteral("user_id"), entry.value(QStringLiteral("user_id")).toString());
    video.insert(QStringLiteral("view_count"), entry.value(QStringLiteral("view_count")).toInt());
    video.insert(QStringLiteral("duration"), entry.value(QStringLiteral("duration")).toString());  // Format "1h23m45s"
    video.insert(QStringLiteral("thumbnail_url"), entry.value(QStringLiteral("thumbnail_url")).toString());
    video.insert(QStringLiteral("created_at"), entry.value(QStringLiteral("created_at")).toString());
    video.insert(QStringLiteral("url"), entry.value(QStringLiteral("url")).toString());
    
    videos.append(video);
  }

  return videos;
}

void TwitchApiClient::handleFollowedChannelsReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleFollowedChannelsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des chaînes suivies"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Followed channels"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("Failed to parse followed channels response"));
    emit followedChannelsReady(QVariantList());
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 followed channels").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No followed channels found in API response"));
    emit followedChannelsReady(QVariantList());
    return;
  }
  
  const QVariantList channels = parseChannelsArray(entries);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Parsed %1 followed channels, fetching user avatars").arg(channels.size()));
  
  // Stocker temporairement les chaînes et récupérer les avatars via /users
  m_pendingChannels = channels;
  
  // Extraire les broadcaster_ids pour récupérer les avatars
  QStringList broadcasterIds;
  for (const QVariant& channelVar : channels) {
    const QVariantMap channel = channelVar.toMap();
    const QString broadcasterId = channel.value(QStringLiteral("broadcaster_id")).toString();
    if (!broadcasterId.isEmpty()) {
      broadcasterIds.append(broadcasterId);
    }
  }
  
  if (!broadcasterIds.isEmpty()) {
    getUsersInfo(broadcasterIds);
  } else {
    // Si pas de broadcaster_ids, émettre directement
    emit followedChannelsReady(channels);
  }
}

void TwitchApiClient::handleTrendingStreamsReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleTrendingStreamsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des streams tendances"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Trending streams"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("Failed to parse trending streams response"));
    emit trendingStreamsReady(QVariantList());
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 trending streams").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No trending streams found in API response"));
    emit trendingStreamsReady(QVariantList());
    return;
  }
  
  const QVariantList streams = parseStreamsArray(entries);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Parsed %1 trending streams, emitting signal").arg(streams.size()));
  emit trendingStreamsReady(streams);
}

void TwitchApiClient::handleNewStreamersReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleNewStreamersReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des nouveaux streamers"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("New streamers"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("Failed to parse new streamers response"));
    emit newStreamersReady(QVariantList());
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 new streamers").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No new streamers found in API response"));
    emit newStreamersReady(QVariantList());
    return;
  }
  
  // Transformer les follows en format streamer
  QVariantList streamers;
  streamers.reserve(entries.size());
  
  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap streamer;
    
    streamer.insert(QStringLiteral("user_id"), entry.value(QStringLiteral("to_id")).toString());
    streamer.insert(QStringLiteral("user_name"), entry.value(QStringLiteral("to_name")).toString());
    streamer.insert(QStringLiteral("followed_at"), entry.value(QStringLiteral("followed_at")).toString());
    
    streamers.append(streamer);
  }
  
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Parsed %1 new streamers, emitting signal").arg(streamers.size()));
  emit newStreamersReady(streamers);
}

void TwitchApiClient::handleCategoryStreamsReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] handleCategoryStreamsReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  
  if (!reply) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("[DEBUG] No reply object"));
    emit categoryStreamsReady(QVariantList());
    return;
  }
  
  int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] HTTP status: %1").arg(httpStatus));
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Response URL: %1").arg(reply->url().toString()));
  
  if (handleNetworkError(reply, QStringLiteral("récupération des streams par catégorie"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Category streams"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Failed to parse category streams response"));
    emit categoryStreamsReady(QVariantList());
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Found %1 category streams in API response").arg(entries.size()));
  
  if (entries.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("[DEBUG] No category streams found in API response (empty data array)"));
    emit categoryStreamsReady(QVariantList());
    return;
  }
  
  const QVariantList streams = parseStreamsArray(entries);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Parsed %1 category streams, emitting signal").arg(streams.size()));
  emit categoryStreamsReady(streams);
}

QVariantList TwitchApiClient::parseChannelsArray(const QJsonArray& entries) {
  QVariantList channels;
  channels.reserve(entries.size());

  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    QVariantMap channel;
    
    const QString broadcasterId = entry.value(QStringLiteral("broadcaster_id")).toString();
    const QString broadcasterName = entry.value(QStringLiteral("broadcaster_name")).toString();
    const QString broadcasterLogin = entry.value(QStringLiteral("broadcaster_login")).toString();
    
    channel.insert(QStringLiteral("broadcaster_id"), broadcasterId);
    channel.insert(QStringLiteral("broadcaster_name"), broadcasterName);
    channel.insert(QStringLiteral("broadcaster_login"), broadcasterLogin);
    channel.insert(QStringLiteral("display_name"), broadcasterName);
    
    // thumbnail_url sera rempli après la requête à /users
    channel.insert(QStringLiteral("thumbnail_url"), QString());
    
    channel.insert(QStringLiteral("game_name"), entry.value(QStringLiteral("game_name")).toString());
    channel.insert(QStringLiteral("game_id"), entry.value(QStringLiteral("game_id")).toString());
    channel.insert(QStringLiteral("is_live"), entry.value(QStringLiteral("is_live")).toBool());
    channel.insert(QStringLiteral("title"), entry.value(QStringLiteral("title")).toString());
    
    channels.append(channel);
  }

  return channels;
}

void TwitchApiClient::getUsersInfo(const QStringList& userIds) {
  if (userIds.isEmpty()) {
    return;
  }
  
  // L'API Twitch accepte jusqu'à 100 IDs par requête
  const int maxIdsPerRequest = 100;
  QStringList limitedIds = userIds.mid(0, maxIdsPerRequest);
  
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/users"));
  QUrlQuery query;
  for (const QString& id : limitedIds) {
    query.addQueryItem(QStringLiteral("id"), id);
  }
  url.setQuery(query);
  
  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting user info for %1 users").arg(limitedIds.size()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleUsersInfoReply);
}

void TwitchApiClient::handleUsersInfoReply() {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("handleUsersInfoReply() called"));
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (handleNetworkError(reply, QStringLiteral("récupération des informations utilisateur"))) {
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = parseJsonResponse(reply, QStringLiteral("Users info"));
  reply->deleteLater();
  if (document.isNull()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("Failed to parse users info response, emitting channels without avatars"));
    emit followedChannelsReady(m_pendingChannels);
    m_pendingChannels.clear();
    return;
  }

  const QJsonObject root = document.object();
  const QJsonArray entries = root.value(QStringLiteral("data")).toArray();
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Found %1 users info").arg(entries.size()));
  
  // Créer une map broadcaster_id -> profile_image_url
  QHash<QString, QString> avatarMap;
  for (const QJsonValue& entryValue : entries) {
    const QJsonObject entry = entryValue.toObject();
    const QString userId = entry.value(QStringLiteral("id")).toString();
    const QString profileImageUrl = entry.value(QStringLiteral("profile_image_url")).toString();
    if (!userId.isEmpty() && !profileImageUrl.isEmpty()) {
      avatarMap.insert(userId, profileImageUrl);
    }
  }
  
  // Mettre à jour les chaînes avec les vraies URLs d'avatar
  for (QVariant& channelVar : m_pendingChannels) {
    QVariantMap channel = channelVar.toMap();
    const QString broadcasterId = channel.value(QStringLiteral("broadcaster_id")).toString();
    if (avatarMap.contains(broadcasterId)) {
      channel.insert(QStringLiteral("thumbnail_url"), avatarMap.value(broadcasterId));
    }
    channelVar = channel;
  }
  
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("Updated %1 channels with avatars, emitting signal").arg(m_pendingChannels.size()));
  emit followedChannelsReady(m_pendingChannels);
  m_pendingChannels.clear();
}

void TwitchApiClient::getFollowedChannels(const QString& userId) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getFollowedChannels() called for userId: %1").arg(userId));
  
  if (m_clientId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Client ID is empty! Cannot request followed channels."));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getFollowedChannels"), QStringLiteral("Client ID manquant"));
    emit errorOccurred(error.toString());
    return;
  }
  
  if (userId.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("No userId provided, emitting empty list"));
    emit followedChannelsReady(QVariantList());
    return;
  }
  
  // Utiliser l'endpoint /channels/followed pour obtenir les chaînes suivies
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/channels/followed"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("user_id"), userId);
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting followed channels from: %1").arg(url.toString()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleFollowedChannelsReply);
}

void TwitchApiClient::getTrendingStreams(int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getTrendingStreams() called with limit: %1").arg(limit));
  
  if (m_clientId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("Client ID is empty! Cannot request trending streams."));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getTrendingStreams"), QStringLiteral("Client ID manquant"));
    emit errorOccurred(error.toString());
    return;
  }
  
  // Utiliser l'endpoint /streams trié par viewers décroissant (par défaut)
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/streams"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("Requesting trending streams from: %1").arg(url.toString()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleTrendingStreamsReply);
}

void TwitchApiClient::getNewFollowedStreamers(const QString& userId, int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("getNewFollowedStreamers() called for userId: %1, limit: %2").arg(userId).arg(limit));
  
  // NOTE: L'endpoint /helix/users/follows est déprécié (erreur 410 "This API is not available")
  // Pour l'instant, on retourne une liste vide
  // TODO: Utiliser /helix/channels/followed à la place qui retourne les chaînes suivies
  core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("getNewFollowedStreamers: endpoint /helix/users/follows is deprecated (410), returning empty list"));
  emit newStreamersReady(QVariantList());
}

void TwitchApiClient::getStreamsByCategory(const QString& gameId, int limit) {
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] getStreamsByCategory() called for gameId: %1, limit: %2").arg(gameId).arg(limit));
  
  if (m_clientId.isEmpty()) {
    core::Logger::error(core::LogCategory::Twitch, QStringLiteral("[DEBUG] Client ID is empty! Cannot request category streams."));
    const auto error = ErrorHandler::twitchApiError(QStringLiteral("getStreamsByCategory"), QStringLiteral("Client ID manquant"));
    emit errorOccurred(error.toString());
    return;
  }
  
  if (gameId.isEmpty()) {
    core::Logger::warning(core::LogCategory::Twitch, QStringLiteral("[DEBUG] No gameId provided, emitting empty list"));
    emit categoryStreamsReady(QVariantList());
    return;
  }
  
  // Utiliser l'endpoint /streams avec game_id pour filtrer par catégorie
  QUrl url(QStringLiteral("https://api.twitch.tv/helix/streams"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("game_id"), gameId);
  query.addQueryItem(QStringLiteral("first"), QString::number(limit));
  url.setQuery(query);

  core::Logger::debug(core::LogCategory::Network, QStringLiteral("[DEBUG] Requesting streams by category from: %1").arg(url.toString()));
  QNetworkReply* reply = getJson(url);
  connect(reply, &QNetworkReply::finished, this, &TwitchApiClient::handleCategoryStreamsReply);
  core::Logger::debug(core::LogCategory::Twitch, QStringLiteral("[DEBUG] getStreamsByCategory() call completed, waiting for reply"));
}

QString TwitchApiClient::expandThumbnail(const QString& templateUrl) const {
  QString sanitized = templateUrl;
  sanitized.replace(QStringLiteral("{width}"), QString::number(core::constants::twitch::kThumbnailWidth));
  sanitized.replace(QStringLiteral("{height}"), QString::number(core::constants::twitch::kThumbnailHeight));
  return sanitized;
}

}  // namespace blueplayer::api::twitch

