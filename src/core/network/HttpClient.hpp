#pragma once

#include "core/Error.hpp"
#include "core/NetworkCache.hpp"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QHash>
#include <QString>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace blueplayer::core::network {

/**
 * @brief Client HTTP centralisé pour toutes les requêtes réseau
 * 
 * Fournit une interface unifiée pour les requêtes HTTP avec:
 * - Gestion centralisée de QNetworkAccessManager avec cache
 * - Support des headers personnalisés et authentification
 * - Gestion d'erreurs standardisée avec la classe Error
 * - Méthodes génériques pour GET, POST, PUT, DELETE
 */
class HttpClient : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructeur
   * @param parent Le parent QObject
   */
  explicit HttpClient(QObject* parent = nullptr);

  /**
   * @brief Destructeur
   */
  ~HttpClient() override = default;

  /**
   * @brief Effectue une requête GET
   * @param url L'URL à requêter
   * @param headers Headers HTTP optionnels
   * @return Le QNetworkReply pour suivre la requête
   */
  QNetworkReply* get(const QUrl& url, const QHash<QString, QString>& headers = {});

  /**
   * @brief Effectue une requête POST
   * @param url L'URL à requêter
   * @param data Les données à envoyer
   * @param headers Headers HTTP optionnels
   * @return Le QNetworkReply pour suivre la requête
   */
  QNetworkReply* post(const QUrl& url, const QByteArray& data = {}, const QHash<QString, QString>& headers = {});

  /**
   * @brief Effectue une requête PUT
   * @param url L'URL à requêter
   * @param data Les données à envoyer
   * @param headers Headers HTTP optionnels
   * @return Le QNetworkReply pour suivre la requête
   */
  QNetworkReply* put(const QUrl& url, const QByteArray& data = {}, const QHash<QString, QString>& headers = {});

  /**
   * @brief Effectue une requête DELETE
   * @param url L'URL à requêter
   * @param headers Headers HTTP optionnels
   * @return Le QNetworkReply pour suivre la requête
   */
  QNetworkReply* deleteResource(const QUrl& url, const QHash<QString, QString>& headers = {});

  /**
   * @brief Configure le token Bearer pour l'authentification
   * @param token Le token Bearer
   */
  void setBearerToken(const QString& token);

  /**
   * @brief Configure un header personnalisé par défaut
   * @param name Le nom du header
   * @param value La valeur du header
   */
  void setDefaultHeader(const QString& name, const QString& value);

  /**
   * @brief Supprime un header par défaut
   * @param name Le nom du header à supprimer
   */
  void removeDefaultHeader(const QString& name);

  /**
   * @brief Efface tous les headers par défaut
   */
  void clearDefaultHeaders();

  /**
   * @brief Vérifie si une réponse réseau contient une erreur
   * @param reply La réponse réseau à vérifier
   * @param context Le contexte de l'erreur pour le message
   * @return Une Error si une erreur est détectée, Error() sinon
   */
  [[nodiscard]] static Error checkNetworkError(QNetworkReply* reply, const QString& context = {});

  /**
   * @brief Obtient le QNetworkAccessManager sous-jacent
   * @return Le QNetworkAccessManager
   */
  [[nodiscard]] QNetworkAccessManager* networkManager() const { return m_networkManager; }
  
  /**
   * @brief Obtient le bearer token actuel
   * @return Le bearer token, ou QString() si non défini
   */
  [[nodiscard]] QString bearerToken() const { return m_bearerToken; }

signals:
  /**
   * @brief Signal émis lorsqu'une erreur réseau survient
   * @param error L'erreur qui s'est produite
   */
  void networkError(const Error& error);

private:
  /**
   * @brief Construit une QNetworkRequest avec les headers par défaut et personnalisés
   * @param url L'URL de la requête
   * @param customHeaders Headers personnalisés additionnels
   * @return La QNetworkRequest configurée
   */
  QNetworkRequest buildRequest(const QUrl& url, const QHash<QString, QString>& customHeaders = {}) const;

  QNetworkAccessManager* m_networkManager = nullptr;
  NetworkCache* m_cache = nullptr;
  QString m_bearerToken;
  QHash<QString, QString> m_defaultHeaders;
};

}  // namespace blueplayer::core::network




