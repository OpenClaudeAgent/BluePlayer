#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace blueplayer::test {

/**
 * @brief Classe helper pour les tests unitaires et d'intégration
 * 
 * Fournit des utilitaires communs pour les tests :
 * - Mocks pour QNetworkAccessManager
 * - Helpers pour créer des réponses JSON mockées
 * - Fixtures pour les données de test
 */
class TestHelpers : public QObject {
  Q_OBJECT

public:
  explicit TestHelpers(QObject* parent = nullptr);

  /**
   * @brief Crée une réponse JSON mockée pour les tests Twitch API
   * @param data Le tableau de données JSON à retourner
   * @return Le JSON formaté comme réponse Twitch API
   */
  static QString createTwitchApiResponse(const QString& data);

  /**
   * @brief Crée une réponse d'erreur Twitch API mockée
   * @param error Le message d'erreur
   * @param statusCode Le code de statut HTTP
   * @return Le JSON formaté comme réponse d'erreur Twitch API
   */
  static QString createTwitchErrorResponse(const QString& error, int statusCode = 400);

  /**
   * @brief Vérifie qu'une URL contient les paramètres attendus
   * @param url L'URL à vérifier
   * @param expectedParams Les paramètres attendus (clé=valeur)
   * @return true si tous les paramètres sont présents
   */
  static bool urlContainsParams(const QUrl& url, const QMap<QString, QString>& expectedParams);

  /**
   * @brief Crée un token OAuth mocké pour les tests
   * @return Un token de test valide
   */
  static QString createMockAccessToken();

  /**
   * @brief Crée un refresh token mocké pour les tests
   * @return Un refresh token de test valide
   */
  static QString createMockRefreshToken();
};

}  // namespace blueplayer::test



