#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

namespace blueplayer::api::twitch {

class TwitchAuthManager;
class TwitchApiClient;

/**
 * @brief Service principal pour l'intégration Twitch
 * 
 * Gère l'authentification et la récupération des streams Twitch.
 * Expose une API simple pour QML avec les propriétés et méthodes nécessaires.
 */
class TwitchService : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
  Q_PROPERTY(QVariantList streams READ streams NOTIFY streamsChanged)
  Q_PROPERTY(QVariantList recommendedStreams READ recommendedStreams NOTIFY recommendedStreamsChanged)
  Q_PROPERTY(QVariantList categories READ categories NOTIFY categoriesChanged)
  Q_PROPERTY(QString selectedStreamUrl READ selectedStreamUrl NOTIFY selectedStreamChanged)
  Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged)

public:
  /**
   * @brief Constructeur
   * @param parent Le parent QObject
   */
  explicit TwitchService(QObject* parent = nullptr);

  /**
   * @brief Vérifie si l'utilisateur est authentifié
   * @return true si authentifié
   */
  [[nodiscard]] bool isAuthenticated() const;

  /**
   * @brief Obtient la liste des streams suivis
   * @return La liste des streams (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList streams() const;

  /**
   * @brief Obtient la liste des streams recommandés
   * @return La liste des streams recommandés (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList recommendedStreams() const;

  /**
   * @brief Obtient la liste des catégories populaires
   * @return La liste des catégories (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList categories() const;

  /**
   * @brief Obtient l'URL du stream sélectionné
   * @return L'URL du stream
   */
  [[nodiscard]] QString selectedStreamUrl() const;

  /**
   * @brief Obtient l'ID utilisateur Twitch
   * @return L'ID utilisateur
   */
  [[nodiscard]] QString userId() const;

  /**
   * @brief Lance le processus d'authentification OAuth
   */
  Q_INVOKABLE void login();

  /**
   * @brief Déconnecte l'utilisateur et efface les tokens
   */
  Q_INVOKABLE void logout();

  /**
   * @brief Rafraîchit la liste des streams suivis
   */
  Q_INVOKABLE void refreshStreams();

  /**
   * @brief Rafraîchit la liste des streams recommandés
   */
  Q_INVOKABLE void refreshRecommendedStreams();

  /**
   * @brief Rafraîchit la liste des catégories populaires
   */
  Q_INVOKABLE void refreshCategories();

  /**
   * @brief Sélectionne et prépare un stream pour la lecture
   * @param index L'index du stream dans la liste
   */
  Q_INVOKABLE void playStream(int index);

signals:
  void authenticatedChanged(bool authenticated);
  void streamsChanged();
  void recommendedStreamsChanged();
  void categoriesChanged();
  void selectedStreamChanged();
  void userIdChanged();
  void errorOccurred(const QString& message);

private:
  void selectUrl(int index);

private slots:
  void onAuthStateChanged(bool authenticated);
  void onAccessTokenChanged(const QString& token);
  void onStreamsReady(const QVariantList& streams);
  void onRecommendedStreamsReady(const QVariantList& streams);
  void onCategoriesReady(const QVariantList& categories);
  void onUserInfoReady(const QString& userId);

private:
  TwitchAuthManager* m_authManager = nullptr;
  TwitchApiClient* m_apiClient = nullptr;
  QVariantList m_streams;
  QVariantList m_recommendedStreams;
  QVariantList m_categories;
  QString m_selectedStreamUrl;
  QString m_userId;
};

}  // namespace blueplayer::api::twitch

