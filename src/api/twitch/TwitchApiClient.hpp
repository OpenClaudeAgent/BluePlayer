#pragma once

#include <QObject>
#include <QVariantList>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace blueplayer::api::twitch {

class TwitchApiClient : public QObject {
  Q_OBJECT

public:
  explicit TwitchApiClient(const QString& clientId, QObject* parent = nullptr);

  void setAccessToken(const QString& token);
  Q_INVOKABLE void listStreams(int limit = 12);

signals:
  void streamsReady(const QVariantList& streams);
  void errorOccurred(const QString& message);

private slots:
  void handleReply();

private:
  QString expandThumbnail(const QString& templateUrl) const;
  QString m_clientId;
  QString m_accessToken;
  QNetworkAccessManager* m_networkManager = nullptr;
};

}  // namespace blueplayer::api::twitch

