#include "core/WatchHistory.hpp"

#include "core/Logger.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QSettings>
#include <QVariantMap>

using blueplayer::core::LogCategory;
using blueplayer::core::Logger;

namespace blueplayer::core {

WatchHistory::WatchHistory(QObject* parent) : QObject(parent) {
  m_settings = new QSettings(this);
  Logger::debug(LogCategory::Core, QStringLiteral("WatchHistory initialized"));
}

WatchHistory::~WatchHistory() = default;

void WatchHistory::saveWatchPosition(const QString& videoId, qint64 position) {
  if (videoId.isEmpty()) {
    Logger::warning(LogCategory::Core,
                    QStringLiteral("Cannot save watch position: videoId is empty"));
    return;
  }

  m_settings->beginGroup(kWatchHistoryGroup);
  m_settings->beginGroup(videoId);
  m_settings->setValue(kPositionKey, position);
  m_settings->setValue(kLastWatchedKey, QDateTime::currentDateTime().toSecsSinceEpoch());
  m_settings->endGroup();
  m_settings->endGroup();
  m_settings->sync();

  Logger::debug(LogCategory::Core,
                QStringLiteral("Saved watch position for video %1: %2 seconds")
                    .arg(videoId)
                    .arg(position));

  emit progressUpdated(videoId);
}

void WatchHistory::saveWatchProgress(const QString& videoId, qint64 position,
                                     qint64 totalDuration) {
  if (videoId.isEmpty()) {
    Logger::warning(LogCategory::Core,
                    QStringLiteral("Cannot save watch progress: videoId is empty"));
    return;
  }

  // Calculer si complété (> 90%)
  bool completed = false;
  if (totalDuration > 0) {
    double percent = (static_cast<double>(position) / static_cast<double>(totalDuration)) * 100.0;
    completed = percent >= 90.0;
  }

  m_settings->beginGroup(kWatchHistoryGroup);
  m_settings->beginGroup(videoId);
  m_settings->setValue(kPositionKey, position);
  m_settings->setValue(kDurationKey, totalDuration);
  m_settings->setValue(kLastWatchedKey, QDateTime::currentDateTime().toSecsSinceEpoch());
  m_settings->setValue(kCompletedKey, completed);
  m_settings->endGroup();
  m_settings->endGroup();
  m_settings->sync();

  Logger::debug(LogCategory::Core,
                QStringLiteral("Saved watch progress for video %1: %2/%3 seconds (%4%)")
                    .arg(videoId)
                    .arg(position)
                    .arg(totalDuration)
                    .arg(totalDuration > 0 ? (position * 100 / totalDuration) : 0));

  emit progressUpdated(videoId);
}

qint64 WatchHistory::getWatchPosition(const QString& videoId) const {
  if (videoId.isEmpty()) {
    return 0;
  }

  m_settings->beginGroup(kWatchHistoryGroup);
  m_settings->beginGroup(videoId);
  const qint64 position = m_settings->value(kPositionKey, 0).toLongLong();
  m_settings->endGroup();
  m_settings->endGroup();

  return position;
}

WatchProgress WatchHistory::getWatchProgress(const QString& videoId) const {
  WatchProgress progress;
  progress.vodId = videoId;
  progress.lastPosition = 0;
  progress.totalDuration = 0;
  progress.completed = false;

  if (videoId.isEmpty()) {
    return progress;
  }

  m_settings->beginGroup(kWatchHistoryGroup);
  m_settings->beginGroup(videoId);

  progress.lastPosition = m_settings->value(kPositionKey, 0).toLongLong();
  progress.totalDuration = m_settings->value(kDurationKey, 0).toLongLong();
  qint64 lastWatchedSecs = m_settings->value(kLastWatchedKey, 0).toLongLong();
  progress.lastWatchedAt = QDateTime::fromSecsSinceEpoch(lastWatchedSecs);
  progress.completed = m_settings->value(kCompletedKey, false).toBool();

  m_settings->endGroup();
  m_settings->endGroup();

  return progress;
}

QVariantList WatchHistory::getVideosInProgress() const {
  QVariantList videos;

  m_settings->beginGroup(kWatchHistoryGroup);
  const QStringList videoIds = m_settings->childGroups();

  for (const QString& videoId : videoIds) {
    m_settings->beginGroup(videoId);
    const qint64 position = m_settings->value(kPositionKey, 0).toLongLong();
    const qint64 duration = m_settings->value(kDurationKey, 0).toLongLong();
    const qint64 lastWatched = m_settings->value(kLastWatchedKey, 0).toLongLong();
    const bool completed = m_settings->value(kCompletedKey, false).toBool();
    m_settings->endGroup();

    // Ne retourner que les VODs avec une position > 0
    if (position > 0) {
      QVariantMap video;
      video.insert(QStringLiteral("videoId"), videoId);
      video.insert(QStringLiteral("position"), position);
      video.insert(QStringLiteral("duration"), duration);
      video.insert(QStringLiteral("lastWatched"), lastWatched);
      video.insert(QStringLiteral("completed"), completed);

      // Calculer le pourcentage
      double percent = 0.0;
      if (duration > 0) {
        percent = (static_cast<double>(position) / static_cast<double>(duration)) * 100.0;
      }
      video.insert(QStringLiteral("progressPercent"), percent);

      videos.append(video);
    }
  }

  m_settings->endGroup();

  Logger::debug(LogCategory::Core,
                QStringLiteral("Found %1 videos in progress").arg(videos.size()));
  return videos;
}

void WatchHistory::clearHistory() {
  m_settings->beginGroup(kWatchHistoryGroup);
  m_settings->remove(QString());  // Supprime tout le groupe
  m_settings->endGroup();
  m_settings->sync();

  Logger::debug(LogCategory::Core, QStringLiteral("Watch history cleared"));
}

void WatchHistory::removeVideo(const QString& videoId) {
  if (videoId.isEmpty()) {
    return;
  }

  m_settings->beginGroup(kWatchHistoryGroup);
  m_settings->remove(videoId);
  m_settings->endGroup();
  m_settings->sync();

  Logger::debug(LogCategory::Core,
                QStringLiteral("Removed video %1 from watch history").arg(videoId));
}

void WatchHistory::markAsCompleted(const QString& videoId) {
  if (videoId.isEmpty()) {
    return;
  }

  m_settings->beginGroup(kWatchHistoryGroup);
  m_settings->beginGroup(videoId);
  m_settings->setValue(kCompletedKey, true);
  m_settings->setValue(kLastWatchedKey, QDateTime::currentDateTime().toSecsSinceEpoch());
  m_settings->endGroup();
  m_settings->endGroup();
  m_settings->sync();

  Logger::debug(LogCategory::Core, QStringLiteral("Marked video %1 as completed").arg(videoId));

  emit progressUpdated(videoId);
}

bool WatchHistory::hasProgress(const QString& videoId) const {
  if (videoId.isEmpty()) {
    return false;
  }

  m_settings->beginGroup(kWatchHistoryGroup);
  bool exists = m_settings->childGroups().contains(videoId);
  if (exists) {
    m_settings->beginGroup(videoId);
    qint64 position = m_settings->value(kPositionKey, 0).toLongLong();
    exists = position > 0;
    m_settings->endGroup();
  }
  m_settings->endGroup();

  return exists;
}

QString WatchHistory::formatLastWatched(const QDateTime& lastWatchedAt) {
  if (!lastWatchedAt.isValid()) {
    return QString();
  }

  QDateTime now = QDateTime::currentDateTime();
  qint64 secsAgo = lastWatchedAt.secsTo(now);

  if (secsAgo < 0) {
    return QString();
  }

  // Moins d'une minute
  if (secsAgo < 60) {
    return QCoreApplication::translate("WatchHistory", "Vu a l'instant");
  }

  // Moins d'une heure
  if (secsAgo < 3600) {
    qint64 minutes = secsAgo / 60;
    if (minutes == 1) {
      return QCoreApplication::translate("WatchHistory", "Vu il y a 1 minute");
    }
    return QCoreApplication::translate("WatchHistory", "Vu il y a %1 minutes").arg(minutes);
  }

  // Moins d'un jour
  if (secsAgo < 86400) {
    qint64 hours = secsAgo / 3600;
    if (hours == 1) {
      return QCoreApplication::translate("WatchHistory", "Vu il y a 1 heure");
    }
    return QCoreApplication::translate("WatchHistory", "Vu il y a %1 heures").arg(hours);
  }

  // Moins d'une semaine
  if (secsAgo < 604800) {
    qint64 days = secsAgo / 86400;
    if (days == 1) {
      return QCoreApplication::translate("WatchHistory", "Vu hier");
    }
    return QCoreApplication::translate("WatchHistory", "Vu il y a %1 jours").arg(days);
  }

  // Moins d'un mois
  if (secsAgo < 2592000) {
    qint64 weeks = secsAgo / 604800;
    if (weeks == 1) {
      return QCoreApplication::translate("WatchHistory", "Vu il y a 1 semaine");
    }
    return QCoreApplication::translate("WatchHistory", "Vu il y a %1 semaines").arg(weeks);
  }

  // Plus d'un mois
  qint64 months = secsAgo / 2592000;
  if (months == 1) {
    return QCoreApplication::translate("WatchHistory", "Vu il y a 1 mois");
  }
  return QCoreApplication::translate("WatchHistory", "Vu il y a %1 mois").arg(months);
}

}  // namespace blueplayer::core
