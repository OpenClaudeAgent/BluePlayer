#include "core/WatchHistory.hpp"

#include "core/Logger.hpp"

#include <QSettings>
#include <QDateTime>
#include <QVariantMap>

using blueplayer::core::Logger;
using blueplayer::core::LogCategory;

namespace blueplayer::core {

WatchHistory::WatchHistory(QObject* parent) : QObject(parent) {
  m_settings = new QSettings(this);
  Logger::debug(LogCategory::Core, QStringLiteral("WatchHistory initialized"));
}

WatchHistory::~WatchHistory() = default;

void WatchHistory::saveWatchPosition(const QString& videoId, qint64 position) {
  if (videoId.isEmpty()) {
    Logger::warning(LogCategory::Core, QStringLiteral("Cannot save watch position: videoId is empty"));
    return;
  }

  m_settings->beginGroup(kWatchHistoryGroup);
  m_settings->beginGroup(videoId);
  m_settings->setValue(kPositionKey, position);
  m_settings->setValue(kLastWatchedKey, QDateTime::currentDateTime().toSecsSinceEpoch());
  m_settings->endGroup();
  m_settings->endGroup();
  m_settings->sync();

  Logger::debug(LogCategory::Core, QStringLiteral("Saved watch position for video %1: %2 seconds")
    .arg(videoId)
    .arg(position));
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

QVariantList WatchHistory::getVideosInProgress() const {
  QVariantList videos;
  
  m_settings->beginGroup(kWatchHistoryGroup);
  const QStringList videoIds = m_settings->childGroups();
  
  for (const QString& videoId : videoIds) {
    m_settings->beginGroup(videoId);
    const qint64 position = m_settings->value(kPositionKey, 0).toLongLong();
    const qint64 lastWatched = m_settings->value(kLastWatchedKey, 0).toLongLong();
    m_settings->endGroup();
    
    // Ne retourner que les VODs avec une position > 0
    if (position > 0) {
      QVariantMap video;
      video.insert(QStringLiteral("videoId"), videoId);
      video.insert(QStringLiteral("position"), position);
      video.insert(QStringLiteral("lastWatched"), lastWatched);
      videos.append(video);
    }
  }
  
  m_settings->endGroup();
  
  Logger::debug(LogCategory::Core, QStringLiteral("Found %1 videos in progress").arg(videos.size()));
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
  
  Logger::debug(LogCategory::Core, QStringLiteral("Removed video %1 from watch history").arg(videoId));
}

}  // namespace blueplayer::core


