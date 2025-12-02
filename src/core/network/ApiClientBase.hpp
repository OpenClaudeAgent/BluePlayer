#pragma once

#include "core/network/HttpClient.hpp"
#include "core/Error.hpp"

#include <QObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrl>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace blueplayer::core::network {

/**
 * @brief Classe de base abstraite pour les clients API
 * 
 * Fournit une interface commune pour tous les clients API avec:
 * - Utilisation de HttpClient pour les requêtes HTTP
 * - Gestion standardisée des erreurs réseau
 * - Parsing JSON générique
 * - Support de l'authentification Bearer token
 */
class ApiClientBase : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructeur
   * @param parent Le parent QObject
   */
  explicit ApiClientBase(QObject* parent = nullptr);

  /**
   * @brief Destructeur
   */
  ~ApiClientBase() override = default;

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

signals:
  /**
   * @brief Signal émis lorsqu'une erreur survient
   * @param error L'erreur qui s'est produite
   */
  void errorOccurred(const Error& error);

protected:
  /**
   * @brief Effectue une requête GET et parse la réponse JSON
   * @param url L'URL à requêter
   * @param headers Headers HTTP optionnels
   * @return Le QNetworkReply pour suivre la requête
   */
  QNetworkReply* getJson(const QUrl& url, const QHash<QString, QString>& headers = {});

  /**
   * @brief Effectue une requête POST avec données JSON
   * @param url L'URL à requêter
   * @param jsonData Les données JSON à envoyer
   * @param headers Headers HTTP optionnels
   * @return Le QNetworkReply pour suivre la requête
   */
  QNetworkReply* postJson(const QUrl& url, const QJsonDocument& jsonData, const QHash<QString, QString>& headers = {});

  /**
   * @brief Effectue une requête POST avec données form-urlencoded
   * @param url L'URL à requêter
   * @param formData Les données form à envoyer
   * @param headers Headers HTTP optionnels
   * @return Le QNetworkReply pour suivre la requête
   */
  QNetworkReply* postForm(const QUrl& url, const QHash<QString, QString>& formData, const QHash<QString, QString>& headers = {});

  /**
   * @brief Parse une réponse JSON depuis un QNetworkReply
   * @param reply La réponse réseau
   * @param errorContext Le contexte pour les messages d'erreur
   * @return Le QJsonDocument parsé, ou QJsonDocument() en cas d'erreur
   */
  QJsonDocument parseJsonResponse(QNetworkReply* reply, const QString& errorContext = {});

  /**
   * @brief Vérifie et gère les erreurs réseau d'une réponse
   * @param reply La réponse réseau à vérifier
   * @param errorContext Le contexte pour les messages d'erreur
   * @return true si une erreur a été détectée et gérée
   */
  bool handleNetworkError(QNetworkReply* reply, const QString& errorContext = {});

  /**
   * @brief Obtient le HttpClient sous-jacent
   * @return Le HttpClient
   */
  [[nodiscard]] HttpClient* httpClient() const { return m_httpClient; }

private slots:
  /**
   * @brief Slot appelé lorsqu'une erreur réseau survient dans HttpClient
   * @param error L'erreur réseau
   */
  void onHttpClientError(const Error& error);

private:
  HttpClient* m_httpClient = nullptr;
};

}  // namespace blueplayer::core::network



