#pragma once

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QVariantMap>

namespace blueplayer::core {

/**
 * @brief Structure représentant les métadonnées d'une VOD en cache
 *
 * Stocke toutes les informations nécessaires pour identifier, afficher et
 * rejouer une VOD enregistrée dans le cache local.
 */
struct VodMetadata {
  QString id;              ///< Identifiant unique (UUID)
  QString streamerLogin;   ///< Login du streamer (ex: "streamer123")
  QString streamerName;    ///< Nom d'affichage du streamer
  QString streamTitle;     ///< Titre du stream
  QDateTime recordedAt;    ///< Date et heure d'enregistrement
  qint64 duration;         ///< Durée en secondes
  qint64 fileSize;         ///< Taille du fichier en octets
  QString filePath;        ///< Chemin vers le fichier vidéo
  QString thumbnailPath;   ///< Chemin vers la miniature (optionnel)
  QString gameCategory;    ///< Catégorie/jeu du stream
  QDateTime lastPlayedAt;  ///< Dernière lecture (pour stratégie LRU)
  qint64 watchPosition;    ///< Position de lecture sauvegardée (secondes)

  /**
   * @brief Génère un nouvel ID unique
   */
  static QString generateId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
  }

  /**
   * @brief Convertit les métadonnées en objet JSON
   */
  [[nodiscard]] QJsonObject toJson() const {
    QJsonObject obj;
    obj[QStringLiteral("id")] = id;
    obj[QStringLiteral("streamerLogin")] = streamerLogin;
    obj[QStringLiteral("streamerName")] = streamerName;
    obj[QStringLiteral("streamTitle")] = streamTitle;
    obj[QStringLiteral("recordedAt")] = recordedAt.toString(Qt::ISODate);
    obj[QStringLiteral("duration")] = duration;
    obj[QStringLiteral("fileSize")] = fileSize;
    obj[QStringLiteral("filePath")] = filePath;
    obj[QStringLiteral("thumbnailPath")] = thumbnailPath;
    obj[QStringLiteral("gameCategory")] = gameCategory;
    obj[QStringLiteral("lastPlayedAt")] = lastPlayedAt.toString(Qt::ISODate);
    obj[QStringLiteral("watchPosition")] = watchPosition;
    return obj;
  }

  /**
   * @brief Crée une instance VodMetadata depuis un objet JSON
   */
  static VodMetadata fromJson(const QJsonObject& obj) {
    VodMetadata meta;
    meta.id = obj[QStringLiteral("id")].toString();
    meta.streamerLogin = obj[QStringLiteral("streamerLogin")].toString();
    meta.streamerName = obj[QStringLiteral("streamerName")].toString();
    meta.streamTitle = obj[QStringLiteral("streamTitle")].toString();
    meta.recordedAt =
        QDateTime::fromString(obj[QStringLiteral("recordedAt")].toString(), Qt::ISODate);
    meta.duration = obj[QStringLiteral("duration")].toInteger();
    meta.fileSize = obj[QStringLiteral("fileSize")].toInteger();
    meta.filePath = obj[QStringLiteral("filePath")].toString();
    meta.thumbnailPath = obj[QStringLiteral("thumbnailPath")].toString();
    meta.gameCategory = obj[QStringLiteral("gameCategory")].toString();
    meta.lastPlayedAt =
        QDateTime::fromString(obj[QStringLiteral("lastPlayedAt")].toString(), Qt::ISODate);
    meta.watchPosition = obj[QStringLiteral("watchPosition")].toInteger();
    return meta;
  }

  /**
   * @brief Convertit les métadonnées en QVariantMap pour l'UI QML
   */
  [[nodiscard]] QVariantMap toVariantMap() const {
    QVariantMap map;
    map[QStringLiteral("id")] = id;
    map[QStringLiteral("streamerLogin")] = streamerLogin;
    map[QStringLiteral("streamerName")] = streamerName;
    map[QStringLiteral("streamTitle")] = streamTitle;
    map[QStringLiteral("recordedAt")] = recordedAt;
    map[QStringLiteral("duration")] = duration;
    map[QStringLiteral("fileSize")] = fileSize;
    map[QStringLiteral("filePath")] = filePath;
    // Ajouter le préfixe file:// pour que QML Image puisse charger le fichier local
    map[QStringLiteral("thumbnailPath")] = thumbnailPath.isEmpty() ? QString() : QStringLiteral("file://") + thumbnailPath;
    map[QStringLiteral("gameCategory")] = gameCategory;
    map[QStringLiteral("lastPlayedAt")] = lastPlayedAt;
    map[QStringLiteral("watchPosition")] = watchPosition;
    
    // Champs formatés pour l'affichage
    map[QStringLiteral("durationFormatted")] = formatDuration(duration);
    map[QStringLiteral("fileSizeFormatted")] = formatFileSize(fileSize);
    map[QStringLiteral("recordedAtFormatted")] = recordedAt.toString(QStringLiteral("dd/MM/yyyy HH:mm"));
    return map;
  }

  /**
   * @brief Formate une durée en secondes en chaîne lisible (ex: "1h30m")
   */
  static QString formatDuration(qint64 seconds) {
    if (seconds < 60) {
      return QString::number(seconds) + QStringLiteral("s");
    }
    if (seconds < 3600) {
      qint64 minutes = seconds / 60;
      qint64 secs = seconds % 60;
      if (secs > 0) {
        return QString::number(minutes) + QStringLiteral("m") + QString::number(secs) + QStringLiteral("s");
      }
      return QString::number(minutes) + QStringLiteral("m");
    }
    qint64 hours = seconds / 3600;
    qint64 minutes = (seconds % 3600) / 60;
    if (minutes > 0) {
      return QString::number(hours) + QStringLiteral("h") + QString::number(minutes) + QStringLiteral("m");
    }
    return QString::number(hours) + QStringLiteral("h");
  }

  /**
   * @brief Formate une taille en octets en chaîne lisible (ex: "1.5 GB")
   */
  static QString formatFileSize(qint64 bytes) {
    constexpr qint64 kKilobyte = 1024;
    constexpr qint64 kMegabyte = kKilobyte * 1024;
    constexpr qint64 kGigabyte = kMegabyte * 1024;

    if (bytes >= kGigabyte) {
      double gb = static_cast<double>(bytes) / static_cast<double>(kGigabyte);
      return QString::number(gb, 'f', 1) + QStringLiteral(" GB");
    }
    if (bytes >= kMegabyte) {
      double mb = static_cast<double>(bytes) / static_cast<double>(kMegabyte);
      return QString::number(mb, 'f', 1) + QStringLiteral(" MB");
    }
    if (bytes >= kKilobyte) {
      double kb = static_cast<double>(bytes) / static_cast<double>(kKilobyte);
      return QString::number(kb, 'f', 0) + QStringLiteral(" KB");
    }
    return QString::number(bytes) + QStringLiteral(" B");
  }

  /**
   * @brief Vérifie si les métadonnées sont valides
   */
  [[nodiscard]] bool isValid() const {
    return !id.isEmpty() && !filePath.isEmpty() && duration > 0;
  }
};

}  // namespace blueplayer::core
