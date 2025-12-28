#include "ui/HomeViewModel.hpp"

#include "core/Constants.hpp"
#include "core/Logger.hpp"

#include <QRegularExpression>
#include <QVariantMap>

using blueplayer::core::Logger;
using blueplayer::core::LogCategory;

namespace blueplayer::ui {

HomeViewModel::HomeViewModel(QObject* parent) : QObject(parent) {
  generatePlaceholderCards();
  updateSectionsData();
}

void HomeViewModel::updateSectionsData() {
  QVariantList sections = createDefaultSections();
  
  // Section 1: Streams suivis
  if (sections.size() > 0) {
    QVariantMap firstSection = sections[0].toMap();
    QVariantList cards = m_followedStreams.isEmpty() ? m_placeholderCards : m_followedStreams;
    firstSection["cards"] = cards;
    sections[0] = firstSection;
  }
  
  // Section 2: Recommandé pour vous (utilise recommendedStreams)
  if (sections.size() > 1) {
    QVariantMap recommendedSection = sections[1].toMap();
    QVariantList recommendedCards = m_recommendedStreams.isEmpty() 
      ? recommendedSection["cards"].toList() 
      : m_recommendedStreams;
    recommendedSection["cards"] = recommendedCards;
    sections[1] = recommendedSection;
  }
  
  // Section 3: Parcourir (catégories)
  if (sections.size() > 2) {
    QVariantMap browseSection = sections[2].toMap();
    QVariantList categoryCards = m_categories.isEmpty() 
      ? browseSection["cards"].toList() 
      : m_categories;
    browseSection["cards"] = categoryCards;
    sections[2] = browseSection;
  }
  
  // Section 4: Clips populaires
  if (sections.size() > 3) {
    QVariantMap popularClipsSection = sections[3].toMap();
    QVariantList clipsCards = m_popularClips.isEmpty() 
      ? popularClipsSection["cards"].toList() 
      : m_popularClips;
    popularClipsSection["cards"] = clipsCards;
    sections[3] = popularClipsSection;
  }
  
  // Section 5: Recommandations par catégorie
  if (sections.size() > 4) {
    QVariantMap categoryStreamsSection = sections[4].toMap();
    Logger::debug(LogCategory::UI, QStringLiteral("Updating section 5: %1 category streams").arg(m_categoryStreams.size()));
    QVariantList categoryStreamsCards = m_categoryStreams.isEmpty() 
      ? categoryStreamsSection["cards"].toList() 
      : m_categoryStreams;
    Logger::debug(LogCategory::UI, QStringLiteral("Section 5 cards: %1").arg(categoryStreamsCards.size()));
    categoryStreamsSection["cards"] = categoryStreamsCards;
    sections[4] = categoryStreamsSection;
  } else {
    Logger::warning(LogCategory::UI, QStringLiteral("Sections size is %1, cannot update section 5").arg(sections.size()));
  }
  
  m_sectionsData = sections;
  emit sectionsDataChanged();
}

void HomeViewModel::refreshTranslations() {
  // Recreate sections with new translations
  updateSectionsData();
}

QVariantList HomeViewModel::transformTwitchStreams(const QVariantList& twitchStreams) {
  if (twitchStreams.isEmpty()) {
    return QVariantList();
  }
  
  QVariantList transformed;
  transformed.reserve(twitchStreams.size());
  
  for (const QVariant& streamVar : twitchStreams) {
    const QVariantMap stream = streamVar.toMap();
    QVariantMap transformedStream;
    
    const QString userName = stream.value(QStringLiteral("user_name")).toString();
    const QString userLogin = stream.value(QStringLiteral("user_login")).toString();
    const QString title = stream.value(QStringLiteral("title")).toString();
    const int viewerCount = stream.value(QStringLiteral("viewer_count")).toInt();
    const QString thumbnailUrl = stream.value(QStringLiteral("thumbnail_url")).toString();
    const QString streamUrl = stream.value(QStringLiteral("stream_url")).toString();
    
    const QString viewerText = QString::number(viewerCount).replace(QRegularExpression("(\\d)(?=(\\d{3})+(?!\\d))"), "\\1 ") + " viewers";
    
    transformedStream[QStringLiteral("name")] = userName;
    transformedStream[QStringLiteral("detail")] = title;
    transformedStream[QStringLiteral("viewers")] = viewerText;
    transformedStream[QStringLiteral("previewImage")] = thumbnailUrl;
    transformedStream[QStringLiteral("streamUrl")] = streamUrl;
    transformedStream[QStringLiteral("streamerLogin")] = userLogin.isEmpty() ? userName.toLower() : userLogin;
    transformedStream[QStringLiteral("userLogin")] = userLogin.isEmpty() ? userName.toLower() : userLogin;
    transformedStream[QStringLiteral("isPlaceholder")] = false;
    
    transformed.append(transformedStream);
  }
  
  return transformed;
}

void HomeViewModel::updateFollowedStreams(const QVariantList& twitchStreams) {
  QVariantList newStreams = transformTwitchStreams(twitchStreams);
  if (areListsEquivalent(m_followedStreams, newStreams, QStringLiteral("streamerLogin"))) {
    Logger::debug(LogCategory::UI, QStringLiteral("Followed streams unchanged, skipping UI update"));
    return;
  }
  m_followedStreams = newStreams;
  emit followedStreamsChanged();
  updateSectionsData();
}

void HomeViewModel::updateRecommendedStreams(const QVariantList& twitchStreams) {
  QVariantList newStreams = transformTwitchStreams(twitchStreams);
  if (areListsEquivalent(m_recommendedStreams, newStreams, QStringLiteral("streamerLogin"))) {
    Logger::debug(LogCategory::UI, QStringLiteral("Recommended streams unchanged, skipping UI update"));
    return;
  }
  m_recommendedStreams = newStreams;
  emit recommendedStreamsChanged();
  updateSectionsData();
}

void HomeViewModel::updateCategories(const QVariantList& twitchCategories) {
  QVariantList transformed;
  transformed.reserve(twitchCategories.size());
  
  for (const QVariant& categoryVar : twitchCategories) {
    const QVariantMap category = categoryVar.toMap();
    QVariantMap transformedCategory;
    
    transformedCategory[QStringLiteral("name")] = category.value(QStringLiteral("name")).toString();
    transformedCategory[QStringLiteral("id")] = category.value(QStringLiteral("id")).toString();
    transformedCategory[QStringLiteral("boxArtUrl")] = category.value(QStringLiteral("boxArtUrl")).toString();
    transformedCategory[QStringLiteral("isPlaceholder")] = false;
    
    transformed.append(transformedCategory);
  }
  
  if (areListsEquivalent(m_categories, transformed, QStringLiteral("id"))) {
    Logger::debug(LogCategory::UI, QStringLiteral("Categories unchanged, skipping UI update"));
    return;
  }
  m_categories = transformed;
  emit categoriesChanged();
  updateSectionsData();
}

QVariantList HomeViewModel::transformClips(const QVariantList& twitchClips) {
  if (twitchClips.isEmpty()) {
    return QVariantList();
  }
  
  QVariantList transformed;
  transformed.reserve(twitchClips.size());
  
  for (const QVariant& clipVar : twitchClips) {
    const QVariantMap clip = clipVar.toMap();
    QVariantMap transformedClip;
    
    const QString title = clip.value(QStringLiteral("title")).toString();
    const QString broadcasterName = clip.value(QStringLiteral("broadcaster_name")).toString();
    const int viewCount = clip.value(QStringLiteral("view_count")).toInt();
    const double duration = clip.value(QStringLiteral("duration")).toDouble();
    const QString thumbnailUrl = clip.value(QStringLiteral("thumbnail_url")).toString();
    const QString url = clip.value(QStringLiteral("url")).toString();
    
    const QString viewText = QString::number(viewCount).replace(QRegularExpression("(\\d)(?=(\\d{3})+(?!\\d))"), "\\1 ");
    const QString durationText = QString::number(static_cast<int>(duration)) + "s";
    
    transformedClip[QStringLiteral("clipTitle")] = title;
    transformedClip[QStringLiteral("broadcasterName")] = broadcasterName;
    transformedClip[QStringLiteral("viewCount")] = viewText;
    transformedClip[QStringLiteral("duration")] = durationText;
    transformedClip[QStringLiteral("thumbnailUrl")] = thumbnailUrl;
    transformedClip[QStringLiteral("url")] = url;
    transformedClip[QStringLiteral("isPlaceholder")] = false;
    
    transformed.append(transformedClip);
  }
  
  return transformed;
}

QVariantList HomeViewModel::transformVideos(const QVariantList& twitchVideos) {
  if (twitchVideos.isEmpty()) {
    return QVariantList();
  }
  
  QVariantList transformed;
  transformed.reserve(twitchVideos.size());
  
  for (const QVariant& videoVar : twitchVideos) {
    const QVariantMap video = videoVar.toMap();
    QVariantMap transformedVideo;
    
    const QString title = video.value(QStringLiteral("title")).toString();
    const QString userName = video.value(QStringLiteral("user_name")).toString();
    const int viewCount = video.value(QStringLiteral("view_count")).toInt();
    const QString duration = video.value(QStringLiteral("duration")).toString();
    const QString thumbnailUrl = video.value(QStringLiteral("thumbnail_url")).toString();
    const QString videoId = video.value(QStringLiteral("id")).toString();
    const QString url = video.value(QStringLiteral("url")).toString();
    
    const QString viewText = QString::number(viewCount).replace(QRegularExpression("(\\d)(?=(\\d{3})+(?!\\d))"), "\\1 ");
    
    transformedVideo[QStringLiteral("videoTitle")] = title;
    transformedVideo[QStringLiteral("userName")] = userName;
    transformedVideo[QStringLiteral("viewCount")] = viewText;
    transformedVideo[QStringLiteral("duration")] = duration;
    transformedVideo[QStringLiteral("thumbnailUrl")] = thumbnailUrl;
    transformedVideo[QStringLiteral("videoId")] = videoId;
    transformedVideo[QStringLiteral("url")] = url;
    transformedVideo[QStringLiteral("hasProgress")] = false;  // TODO: Vérifier avec WatchHistory
    transformedVideo[QStringLiteral("watchPosition")] = 0;  // TODO: Récupérer depuis WatchHistory
    transformedVideo[QStringLiteral("isPlaceholder")] = false;
    
    transformed.append(transformedVideo);
  }
  
  return transformed;
}

QVariantList HomeViewModel::transformChannels(const QVariantList& twitchChannels) {
  if (twitchChannels.isEmpty()) {
    return QVariantList();
  }
  
  QVariantList transformed;
  transformed.reserve(twitchChannels.size());
  
  for (const QVariant& channelVar : twitchChannels) {
    const QVariantMap channel = channelVar.toMap();
    QVariantMap transformedChannel;
    
    const QString channelName = channel.value(QStringLiteral("broadcaster_name")).toString();
    const QString displayName = channel.value(QStringLiteral("display_name")).toString();
    QString thumbnailUrl = channel.value(QStringLiteral("thumbnail_url")).toString();
    const QString broadcasterLogin = channel.value(QStringLiteral("broadcaster_login")).toString();
    const bool isLive = channel.value(QStringLiteral("is_live")).toBool();
    const QString gameName = channel.value(QStringLiteral("game_name")).toString();
    
    // Si thumbnail_url est vide, construire l'URL de l'avatar de profil à partir du login
    if (thumbnailUrl.isEmpty() && !broadcasterLogin.isEmpty()) {
      thumbnailUrl = QStringLiteral("https://static-cdn.jtvnw.net/jtv_user_pictures/%1-profile_image-300x300.png").arg(broadcasterLogin);
    }
    
    transformedChannel[QStringLiteral("channelName")] = channelName;
    transformedChannel[QStringLiteral("displayName")] = displayName;
    transformedChannel[QStringLiteral("thumbnailUrl")] = thumbnailUrl;
    transformedChannel[QStringLiteral("isLive")] = isLive;
    transformedChannel[QStringLiteral("gameName")] = gameName;
    transformedChannel[QStringLiteral("isPlaceholder")] = thumbnailUrl.isEmpty();
    
    transformed.append(transformedChannel);
  }
  
  return transformed;
}

void HomeViewModel::updatePopularClips(const QVariantList& twitchClips) {
  QVariantList newClips = transformClips(twitchClips);
  if (areListsEquivalent(m_popularClips, newClips, QStringLiteral("url"))) {
    Logger::debug(LogCategory::UI, QStringLiteral("Popular clips unchanged, skipping UI update"));
    return;
  }
  m_popularClips = newClips;
  Logger::debug(LogCategory::UI, QStringLiteral("Popular clips updated: %1").arg(m_popularClips.size()));
  emit popularClipsChanged();
  updateSectionsData();
}

void HomeViewModel::updateFollowedClips(const QVariantList& twitchClips) {
  QVariantList newClips = transformClips(twitchClips);
  if (areListsEquivalent(m_followedClips, newClips, QStringLiteral("url"))) {
    Logger::debug(LogCategory::UI, QStringLiteral("Followed clips unchanged, skipping UI update"));
    return;
  }
  m_followedClips = newClips;
  Logger::debug(LogCategory::UI, QStringLiteral("Followed clips updated: %1").arg(m_followedClips.size()));
  emit followedClipsChanged();
  updateSectionsData();
}

void HomeViewModel::updateVideos(const QVariantList& twitchVideos) {
  QVariantList newVideos = transformVideos(twitchVideos);
  if (areListsEquivalent(m_videos, newVideos, QStringLiteral("videoId"))) {
    Logger::debug(LogCategory::UI, QStringLiteral("Videos unchanged, skipping UI update"));
    return;
  }
  m_videos = newVideos;
  Logger::debug(LogCategory::UI, QStringLiteral("Videos updated: %1").arg(m_videos.size()));
  emit videosChanged();
  updateSectionsData();
}

void HomeViewModel::updateFollowedChannels(const QVariantList& twitchChannels) {
  QVariantList newChannels = transformChannels(twitchChannels);
  if (areListsEquivalent(m_followedChannels, newChannels, QStringLiteral("channelName"))) {
    Logger::debug(LogCategory::UI, QStringLiteral("Followed channels unchanged, skipping UI update"));
    return;
  }
  m_followedChannels = newChannels;
  Logger::debug(LogCategory::UI, QStringLiteral("Followed channels updated: %1").arg(m_followedChannels.size()));
  emit followedChannelsChanged();
  updateSectionsData();
}

void HomeViewModel::updateNewStreamers(const QVariantList& twitchStreamers) {
  // Les nouveaux streamers sont déjà dans le format streamer, on les transforme en streams
  QVariantList transformed;
  transformed.reserve(twitchStreamers.size());
  
  for (const QVariant& streamerVar : twitchStreamers) {
    const QVariantMap streamer = streamerVar.toMap();
    QVariantMap transformedStreamer;
    
    transformedStreamer[QStringLiteral("user_name")] = streamer.value(QStringLiteral("user_name")).toString();
    transformedStreamer[QStringLiteral("title")] = tr("New followed streamer");
    transformedStreamer[QStringLiteral("viewer_count")] = 0;
    transformedStreamer[QStringLiteral("thumbnail_url")] = QString();
    transformedStreamer[QStringLiteral("stream_url")] = QStringLiteral("https://www.twitch.tv/%1").arg(streamer.value(QStringLiteral("user_name")).toString());
    
    transformed.append(transformedStreamer);
  }
  
  m_newStreamers = transformTwitchStreams(transformed);
  emit newStreamersChanged();
  updateSectionsData();
}

void HomeViewModel::updateCategoryStreams(const QVariantList& twitchStreams) {
  QVariantList newStreams = transformTwitchStreams(twitchStreams);
  if (areListsEquivalent(m_categoryStreams, newStreams, QStringLiteral("streamerLogin"))) {
    Logger::debug(LogCategory::UI, QStringLiteral("Category streams unchanged, skipping UI update"));
    return;
  }
  m_categoryStreams = newStreams;
  Logger::debug(LogCategory::UI, QStringLiteral("Category streams updated: %1").arg(m_categoryStreams.size()));
  emit categoryStreamsChanged();
  updateSectionsData();
}

void HomeViewModel::generatePlaceholderCards() {
  m_placeholderCards.clear();
  m_placeholderCards.reserve(blueplayer::core::constants::ui::kPlaceholderCardsCount);
  
  for (int i = 0; i < blueplayer::core::constants::ui::kPlaceholderCardsCount; ++i) {
    QVariantMap card;
    card[QStringLiteral("isPlaceholder")] = true;
    m_placeholderCards.append(card);
  }
  
  Logger::debug(LogCategory::UI, QStringLiteral("Generated %1 placeholder cards").arg(m_placeholderCards.size()));
}

QVariantList HomeViewModel::createDefaultSections() const {
  QVariantList sections;
  
  // Section 1: Followed streams (populated dynamically)
  QVariantMap section1;
  section1[QStringLiteral("title")] = tr("Your followed streamers");
  section1[QStringLiteral("subtitle")] = tr("Live channels");
  section1[QStringLiteral("cards")] = m_placeholderCards;  // Default placeholders, replaced in updateSectionsData()
  sections.append(section1);
  
  // Section 2: Recommended for you (uses m_recommendedStreams)
  QVariantMap section2;
  section2[QStringLiteral("title")] = tr("Recommended for you");
  section2[QStringLiteral("subtitle")] = tr("Based on your preferences");
  section2[QStringLiteral("type")] = QStringLiteral("streams");
  section2[QStringLiteral("cards")] = m_placeholderCards;  // Replaced with real data in updateSectionsData()
  sections.append(section2);
  
  // Section 3: Browse (categories)
  QVariantMap section3;
  section3[QStringLiteral("title")] = tr("Browse");
  section3[QStringLiteral("subtitle")] = tr("Discover popular categories");
  QVariantList categoryCards;
  // Default cards if no categories loaded
  categoryCards.append(createCategoryCard("Just Chatting", "509658", ""));
  categoryCards.append(createCategoryCard("League of Legends", "21779", ""));
  categoryCards.append(createCategoryCard("Fortnite", "33214", ""));
  categoryCards.append(createCategoryCard("VALORANT", "516575", ""));
  categoryCards.append(createCategoryCard("Minecraft", "27471", ""));
  categoryCards.append(createCategoryCard("GTA V", "32982", ""));
  section3[QStringLiteral("cards")] = categoryCards;
  sections.append(section3);
  
  // Section 4: Popular clips
  QVariantMap section4;
  section4[QStringLiteral("title")] = tr("Popular clips");
  section4[QStringLiteral("subtitle")] = tr("Best moments");
  section4[QStringLiteral("type")] = QStringLiteral("clips");
  section4[QStringLiteral("cards")] = m_placeholderCards;
  sections.append(section4);
  
  // Section 5: Category recommendations (dynamic)
  QVariantMap section5;
  section5[QStringLiteral("title")] = tr("Category recommendations");
  section5[QStringLiteral("subtitle")] = tr("Discover by game");
  section5[QStringLiteral("type")] = QStringLiteral("streams");
  section5[QStringLiteral("cards")] = m_placeholderCards;
  sections.append(section5);
  
  return sections;
}

QVariantMap HomeViewModel::createCard(const QString& name, const QString& detail, const QString& viewers) const {
  QVariantMap card;
  card[QStringLiteral("name")] = name;
  card[QStringLiteral("detail")] = detail;
  card[QStringLiteral("viewers")] = viewers;
  card[QStringLiteral("previewImage")] = QString();  // Pas d'image pour les cartes par défaut
  card[QStringLiteral("isPlaceholder")] = true;  // Marquer comme placeholder car pas d'image
  return card;
}

QVariantMap HomeViewModel::createCategoryCard(const QString& name, const QString& id, const QString& boxArtUrl) const {
  QVariantMap card;
  card[QStringLiteral("name")] = name;
  card[QStringLiteral("id")] = id;
  card[QStringLiteral("boxArtUrl")] = boxArtUrl;
  card[QStringLiteral("isPlaceholder")] = boxArtUrl.isEmpty();
  return card;
}

bool HomeViewModel::areListsEquivalent(const QVariantList& oldList, const QVariantList& newList, const QString& keyField) {
  if (oldList.size() != newList.size()) {
    return false;
  }
  
  for (int i = 0; i < oldList.size(); ++i) {
    const QVariantMap oldItem = oldList[i].toMap();
    const QVariantMap newItem = newList[i].toMap();
    
    // Compare by key field (e.g., streamerLogin, id)
    if (oldItem.value(keyField) != newItem.value(keyField)) {
      return false;
    }
    
    // For streams, also check viewer count changes (live status indicator)
    if (keyField == QStringLiteral("streamerLogin") || keyField == QStringLiteral("userLogin")) {
      if (oldItem.value(QStringLiteral("viewers")) != newItem.value(QStringLiteral("viewers"))) {
        return false;
      }
    }
  }
  
  return true;
}

}  // namespace blueplayer::ui

