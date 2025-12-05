#include "HlsAdFilter.hpp"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>

namespace blueplayer::media {

HlsAdFilter::HlsAdFilter(QObject* parent) : QObject(parent) {
  m_networkManager = new QNetworkAccessManager(this);
  m_adCheckTimer = new QTimer(this);
  m_adCheckTimer->setInterval(AD_CHECK_INTERVAL_MS);
  connect(m_adCheckTimer, &QTimer::timeout, this, &HlsAdFilter::checkForAds);
}

HlsAdFilter::~HlsAdFilter() {
  stop();
}

void HlsAdFilter::startFiltering(const QString& masterPlaylistUrl, const QString& preferredQuality) {
  // Extraire le login du streamer de l'URL pour tracker si c'est le même stream
  QString streamerLogin = extractStreamerLogin(masterPlaylistUrl);
  
  // Ne réinitialiser le compteur QUE si c'est un NOUVEAU streamer
  bool isNewStream = (streamerLogin != m_currentStreamerLogin);
  
  if (isNewStream) {
    emit debugLog(QStringLiteral("[AdFilter] 🎬 NEW stream: %1 - resetting").arg(streamerLogin));
    m_retryCount = 0;
    m_currentStreamerLogin = streamerLogin;
  } else {
    emit debugLog(QStringLiteral("[AdFilter] 🔄 Same stream (%1), retry count: %2").arg(streamerLogin).arg(m_retryCount));
  }
  
  m_masterPlaylistUrl = masterPlaylistUrl;
  m_preferredQuality = preferredQuality;
  m_adsDetected = false;
  m_adSegmentCount = 0;
  
  fetchMasterPlaylist(masterPlaylistUrl);
}

QString HlsAdFilter::extractStreamerLogin(const QString& url) {
  // URL format: https://usher.ttvnw.net/api/channel/hls/STREAMER.m3u8?...
  QRegularExpression re(QStringLiteral("/hls/([^.]+)\\.m3u8"));
  QRegularExpressionMatch match = re.match(url);
  if (match.hasMatch()) {
    return match.captured(1);
  }
  return QString();
}

void HlsAdFilter::stop() {
  m_adCheckTimer->stop();
}

void HlsAdFilter::fetchMasterPlaylist(const QString& url) {
  emit debugLog(QStringLiteral("[AdFilter] Fetching master playlist..."));
  
  QNetworkRequest request;
  request.setUrl(QUrl(url));
  request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0"));
  
  QNetworkReply* reply = m_networkManager->get(request);
  connect(reply, &QNetworkReply::finished, this, &HlsAdFilter::onMasterPlaylistReceived);
}

void HlsAdFilter::onMasterPlaylistReceived() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) return;
  reply->deleteLater();
  
  if (reply->error() != QNetworkReply::NoError) {
    emit error(QStringLiteral("Failed to fetch master playlist: %1").arg(reply->errorString()));
    return;
  }
  
  QString content = QString::fromUtf8(reply->readAll());
  emit debugLog(QStringLiteral("[AdFilter] Master playlist received (%1 bytes)").arg(content.size()));
  
  // Parser les variantes
  QList<StreamVariant> variants = parseVariants(content);
  
  if (variants.isEmpty()) {
    emit error(QStringLiteral("No stream variants found in master playlist"));
    return;
  }
  
  emit debugLog(QStringLiteral("[AdFilter] Found %1 quality variants").arg(variants.size()));
  
  // Sélectionner la meilleure variante
  m_selectedVariantUrl = selectBestVariant(variants, m_preferredQuality);
  
  if (m_selectedVariantUrl.isEmpty()) {
    emit error(QStringLiteral("Could not select a stream variant"));
    return;
  }
  
  // Vérifier le variant playlist pour les pubs
  fetchVariantPlaylist(m_selectedVariantUrl);
}

void HlsAdFilter::fetchVariantPlaylist(const QString& url) {
  emit debugLog(QStringLiteral("[AdFilter] Fetching variant playlist to check for ads..."));
  
  QNetworkRequest request;
  request.setUrl(QUrl(url));
  request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0"));
  
  QNetworkReply* reply = m_networkManager->get(request);
  connect(reply, &QNetworkReply::finished, this, &HlsAdFilter::onVariantPlaylistReceived);
}

void HlsAdFilter::onVariantPlaylistReceived() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) return;
  reply->deleteLater();
  
  if (reply->error() != QNetworkReply::NoError) {
    emit error(QStringLiteral("Failed to fetch variant playlist: %1").arg(reply->errorString()));
    // En cas d'erreur, émettre quand même l'URL pour tenter la lecture
    emit cleanStreamReady(m_selectedVariantUrl);
    return;
  }
  
  QString content = QString::fromUtf8(reply->readAll());
  emit debugLog(QStringLiteral("[AdFilter] Variant playlist size: %1 bytes").arg(content.size()));
  
  // Détecter les pubs
  bool hasAds = detectAdsInPlaylist(content);
  
  if (hasAds) {
    m_adsDetected = true;
    emit debugLog(QStringLiteral("[AdFilter] ⚠️  ADS DETECTED! %1 ad markers found").arg(m_adSegmentCount));
    emit adsDetected(m_adSegmentCount);
    
    // Stratégie simplifiée: essayer 1-2 fois avec nouveau token, puis jouer quand même
    // Les pubs Twitch sont server-side stitched, donc nouveau token != stream propre garanti
    if (m_retryCount < MAX_RETRIES) {
      m_retryCount++;
      emit debugLog(QStringLiteral("[AdFilter] 🔄 Trying NEW token (attempt %1/%2)...").arg(m_retryCount).arg(MAX_RETRIES));
      
      // Signal pour demander un nouveau token
      QTimer::singleShot(300, this, [this]() {
        emit requestNewToken();
      });
      return;
    }
    
    // Max retries atteint - JOUER QUAND MÊME
    // Les pubs durent généralement 15-30 secondes, le stream commencera après
    emit debugLog(QStringLiteral("[AdFilter] ⏩ Playing stream (ads will play first, ~15-30 seconds)"));
    emit maxRetriesReached(m_selectedVariantUrl);
    
    // Monitoring pour notifier quand les pubs sont finies
    m_adCheckTimer->start();
    return;
  }
  
  // Pas de pubs - stream propre!
  m_adsDetected = false;
  m_retryCount = 0;  // Reset pour le prochain stream
  emit debugLog(QStringLiteral("[AdFilter] ✅ CLEAN STREAM! No ads detected."));
  
  if (m_adCheckTimer->isActive()) {
    m_adCheckTimer->stop();
    emit adsFinished();
  }
  
  // Émettre l'URL propre
  emit cleanStreamReady(m_selectedVariantUrl);
}

void HlsAdFilter::checkForAds() {
  // Re-vérifier le playlist pour voir si les pubs sont terminées
  // Ne pas re-trigger maxRetriesReached, juste vérifier
  if (!m_selectedVariantUrl.isEmpty()) {
    emit debugLog(QStringLiteral("[AdFilter] 🔍 Checking if ads are finished..."));
    
    QNetworkRequest request;
    request.setUrl(QUrl(m_selectedVariantUrl));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0"));
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
      reply->deleteLater();
      
      if (reply->error() != QNetworkReply::NoError) {
        return;
      }
      
      QString content = QString::fromUtf8(reply->readAll());
      bool hasAds = detectAdsInPlaylist(content);
      
      if (!hasAds && m_adsDetected) {
        // Les pubs sont finies!
        m_adsDetected = false;
        m_adCheckTimer->stop();
        emit debugLog(QStringLiteral("[AdFilter] ✅ ADS FINISHED! Stream is now clean."));
        emit adsFinished();
      } else if (hasAds) {
        emit debugLog(QStringLiteral("[AdFilter] ⏳ Still detecting %1 ad markers...").arg(m_adSegmentCount));
      }
    });
  }
}

QList<HlsAdFilter::StreamVariant> HlsAdFilter::parseVariants(const QString& playlistContent) {
  QList<StreamVariant> variants;
  QStringList lines = playlistContent.split('\n');
  
  for (int i = 0; i < lines.size(); ++i) {
    QString line = lines[i].trimmed();
    
    if (line.startsWith(QStringLiteral("#EXT-X-STREAM-INF:"))) {
      StreamVariant variant;
      
      // Parser BANDWIDTH
      QRegularExpression bandwidthRe(QStringLiteral("BANDWIDTH=(\\d+)"));
      QRegularExpressionMatch match = bandwidthRe.match(line);
      if (match.hasMatch()) {
        variant.bandwidth = match.captured(1).toInt();
      }
      
      // Parser RESOLUTION
      QRegularExpression resolutionRe(QStringLiteral("RESOLUTION=(\\d+)x(\\d+)"));
      match = resolutionRe.match(line);
      if (match.hasMatch()) {
        variant.width = match.captured(1).toInt();
        variant.height = match.captured(2).toInt();
      }
      
      // Parser VIDEO (nom de qualité)
      QRegularExpression videoRe(QStringLiteral("VIDEO=\"([^\"]+)\""));
      match = videoRe.match(line);
      if (match.hasMatch()) {
        variant.name = match.captured(1);
      }
      
      // URL sur la ligne suivante
      if (i + 1 < lines.size()) {
        QString nextLine = lines[i + 1].trimmed();
        if (!nextLine.isEmpty() && !nextLine.startsWith('#')) {
          variant.url = nextLine;
          variants.append(variant);
          
          emit debugLog(QStringLiteral("[AdFilter] Variant: %1 (%2x%3) bandwidth=%4")
                       .arg(variant.name)
                       .arg(variant.width)
                       .arg(variant.height)
                       .arg(variant.bandwidth));
        }
      }
    }
  }
  
  return variants;
}

QString HlsAdFilter::selectBestVariant(const QList<StreamVariant>& variants, const QString& preferredQuality) {
  if (variants.isEmpty()) return QString();
  
  // Priorités de qualité
  QStringList priorities;
  if (preferredQuality == "chunked" || preferredQuality == "source") {
    priorities = {"chunked", "1080p60", "1080p", "720p60", "720p", "480p", "360p"};
  } else if (preferredQuality.contains("1080")) {
    priorities = {"1080p60", "1080p", "chunked", "720p60", "720p"};
  } else {
    priorities = {"chunked", "1080p60", "1080p", "720p60", "720p", "480p"};
  }
  
  // Chercher par priorité
  for (const QString& quality : priorities) {
    for (const StreamVariant& variant : variants) {
      if (variant.name.contains(quality, Qt::CaseInsensitive)) {
        emit debugLog(QStringLiteral("[AdFilter] Selected quality: %1").arg(variant.name));
        return variant.url;
      }
    }
  }
  
  // Fallback: premier variant (plus haute qualité généralement)
  emit debugLog(QStringLiteral("[AdFilter] Fallback to first variant: %1").arg(variants.first().name));
  return variants.first().url;
}

bool HlsAdFilter::detectAdsInPlaylist(const QString& playlistContent) {
  m_adSegmentCount = 0;
  
  // Patterns qui indiquent des pubs Twitch
  // Source: https://github.com/pixeltris/TwitchAdSolutions
  
  QStringList adPatterns = {
    // Tags Twitch pour les pubs stitchées
    QStringLiteral("twitch-stitched-ad"),
    QStringLiteral("twitch-prefetch"),
    QStringLiteral("Amazon-Ads"),
    QStringLiteral("stitched-ad"),
    
    // EXT-X-DATERANGE avec classe pub
    QStringLiteral("CLASS=\"twitch-stitched-ad\""),
    QStringLiteral("X-TV-TWITCH-AD-"),
    
    // Segments pre-roll
    QStringLiteral("EXT-X-DISCONTINUITY"),
    
    // Attributs spécifiques aux pubs
    QStringLiteral("AD-INSERTION"),
    QStringLiteral("SCTE35-OUT"),
  };
  
  bool hasAds = false;
  
  for (const QString& pattern : adPatterns) {
    if (playlistContent.contains(pattern, Qt::CaseInsensitive)) {
      hasAds = true;
      
      // Compter les occurrences
      int count = 0;
      int pos = 0;
      while ((pos = playlistContent.indexOf(pattern, pos, Qt::CaseInsensitive)) != -1) {
        ++count;
        ++pos;
      }
      m_adSegmentCount += count;
      
      emit debugLog(QStringLiteral("[AdFilter] Ad marker found: '%1' (x%2)").arg(pattern).arg(count));
    }
  }
  
  // Vérification supplémentaire: segments avec durée courte (< 2s) au début peuvent être des pubs
  QRegularExpression extinf(QStringLiteral("#EXTINF:([\\d.]+),"));
  QRegularExpressionMatchIterator it = extinf.globalMatch(playlistContent);
  
  int shortSegments = 0;
  int segmentIndex = 0;
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    double duration = match.captured(1).toDouble();
    
    // Les premiers segments courts peuvent indiquer un pre-roll
    if (segmentIndex < 5 && duration < 2.0) {
      shortSegments++;
    }
    segmentIndex++;
  }
  
  if (shortSegments >= 3) {
    emit debugLog(QStringLiteral("[AdFilter] Suspicious: %1 short segments at start (possible pre-roll)").arg(shortSegments));
    hasAds = true;
  }
  
  return hasAds;
}

QString HlsAdFilter::filterAdsFromPlaylist(const QString& playlistContent) {
  // Pour une implémentation future: filtrer les segments pub du playlist
  // Nécessite un proxy HTTP local pour servir le playlist modifié à FFmpeg
  
  // Pour l'instant, on retourne le playlist original
  return playlistContent;
}

}  // namespace blueplayer::media

