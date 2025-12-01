#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

namespace blueplayer::api::twitch {

class TwitchAuthManager;
class TwitchApiClient;

class TwitchService : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
  Q_PROPERTY(QVariantList streams READ streams NOTIFY streamsChanged)
  Q_PROPERTY(QString selectedStreamUrl READ selectedStreamUrl NOTIFY selectedStreamChanged)

public:
  explicit TwitchService(QObject* parent = nullptr);

  [[nodiscard]] bool isAuthenticated() const;
  [[nodiscard]] QVariantList streams() const;
  [[nodiscard]] QString selectedStreamUrl() const;

  Q_INVOKABLE void login();
  Q_INVOKABLE void logout();
  Q_INVOKABLE void refreshStreams();
  Q_INVOKABLE void playStream(int index);

signals:
  void authenticatedChanged(bool authenticated);
  void streamsChanged();
  void selectedStreamChanged();
  void errorOccurred(const QString& message);

private:
  void selectUrl(int index);

private slots:
  void onAuthStateChanged(bool authenticated);
  void onAccessTokenChanged(const QString& token);
  void onStreamsReady(const QVariantList& streams);

private:
  TwitchAuthManager* m_authManager = nullptr;
  TwitchApiClient* m_apiClient = nullptr;
  QVariantList m_streams;
  QString m_selectedStreamUrl;
};

}  // namespace blueplayer::api::twitch

