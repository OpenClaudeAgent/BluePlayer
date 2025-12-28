#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <functional>

// Forward declaration for unit testing
class TestHlsAdFilter;

namespace blueplayer::media {

/**
 * @brief Filtre HLS pour supprimer les segments publicitaires Twitch
 *
 * Inspiré par TwitchAdSolutions vaft
 * (https://github.com/pixeltris/TwitchAdSolutions)
 *
 * Stratégie VAFT (Video Ad-Free Token):
 * 1. Récupère le master playlist et sélectionne la meilleure qualité
 * 2. Vérifie le variant playlist pour les marqueurs de pub
 * 3. Si pubs détectées -> DEMANDE UN NOUVEAU TOKEN (pas juste retry le même
 * playlist)
 * 4. Répète jusqu'à obtenir un stream propre ou timeout
 */
class HlsAdFilter : public QObject {
  Q_OBJECT

public:
  explicit HlsAdFilter(QObject *parent = nullptr);
  ~HlsAdFilter() override;

  /**
   * @brief Démarre le filtrage d'un stream
   * @param masterPlaylistUrl URL du master playlist HLS
   * @param preferredQuality Qualité préférée (chunked, 1080p60, etc.)
   */
  void startFiltering(const QString &masterPlaylistUrl,
                      const QString &preferredQuality = "chunked");

  /**
   * @brief Arrête le filtrage
   */
  void stop();

  /**
   * @brief Vérifie si des pubs sont en cours
   */
  bool hasAds() const { return m_adsDetected; }

  /**
   * @brief Nombre de segments pub détectés
   */
  int adSegmentCount() const { return m_adSegmentCount; }

  /**
   * @brief Nombre de retry effectués
   */
  int retryCount() const { return m_retryCount; }

signals:
  /**
   * @brief Émis quand une URL propre (sans pub) est disponible
   */
  void cleanStreamReady(const QString &variantUrl);

  /**
   * @brief Émis quand des pubs sont détectées et qu'on veut un nouveau token
   * C'est le signal clé de la stratégie VAFT
   */
  void requestNewToken();

  /**
   * @brief Émis quand des pubs sont détectées (info seulement)
   */
  void adsDetected(int segmentCount);

  /**
   * @brief Émis quand les pubs sont terminées
   */
  void adsFinished();

  /**
   * @brief Émis après MAX_RETRIES sans succès - on joue quand même
   */
  void maxRetriesReached(const QString &variantUrl);

  /**
   * @brief Émis en cas d'erreur
   */
  void error(const QString &message);

  /**
   * @brief Log de debug
   */
  void debugLog(const QString &message);

private slots:
  void onMasterPlaylistReceived();
  void onVariantPlaylistReceived();
  void checkForAds();

private:
  struct StreamVariant {
    QString name;
    QString url;
    int bandwidth = 0;
    int width = 0;
    int height = 0;
  };

  void fetchMasterPlaylist(const QString &url);
  void fetchVariantPlaylist(const QString &url);
  QList<StreamVariant> parseVariants(const QString &playlistContent);
  QString selectBestVariant(const QList<StreamVariant> &variants,
                            const QString &preferredQuality);
  bool detectAdsInPlaylist(const QString &playlistContent);
  QString filterAdsFromPlaylist(const QString &playlistContent);
  QString extractStreamerLogin(const QString &url);

  // Friend class for unit testing private methods
  friend class ::TestHlsAdFilter;

  QNetworkAccessManager *m_networkManager = nullptr;
  QTimer *m_adCheckTimer = nullptr;

  QString m_masterPlaylistUrl;
  QString m_selectedVariantUrl;
  QString m_preferredQuality;
  QString m_currentStreamerLogin; // Pour tracker si c'est le même stream

  bool m_adsDetected = false;
  int m_adSegmentCount = 0;
  int m_retryCount = 0;
  static constexpr int MAX_RETRIES = 15; // More attempts for ad-free token
  static constexpr int AD_CHECK_INTERVAL_MS = 2000;
};

} // namespace blueplayer::media
