#include "api/twitch/TwitchApiClient.hpp"

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

  QNetworkRequest request(url);
  request.setRawHeader("Client-Id", m_clientId.toUtf8());
  request.setRawHeader("Authorization",
                       QStringLiteral("Bearer %1").arg(m_accessToken).toUtf8());

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

QString TwitchApiClient::expandThumbnail(const QString& templateUrl) const {
  QString sanitized = templateUrl;
  sanitized.replace(QStringLiteral("{width}"), QStringLiteral("320"));
  sanitized.replace(QStringLiteral("{height}"), QStringLiteral("180"));
  return sanitized;
}

}  // namespace blueplayer::api::twitch

