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
  
  // Section 5: Clips populaires
  if (sections.size() > 4) {
    QVariantMap popularClipsSection = sections[4].toMap();
    QVariantList clipsCards = m_popularClips.isEmpty() 
      ? popularClipsSection["cards"].toList() 
      : m_popularClips;
    popularClipsSection["cards"] = clipsCards;
    sections[4] = popularClipsSection;
  }
  
  // Section 6: Recommandations par catégorie
  if (sections.size() > 5) {
    QVariantMap categoryStreamsSection = sections[5].toMap();
    Logger::debug(LogCategory::UI, QStringLiteral("Updating section 6: %1 category streams").arg(m_categoryStreams.size()));
    QVariantList categoryStreamsCards = m_categoryStreams.isEmpty() 
      ? categoryStreamsSection["cards"].toList() 
      : m_categoryStreams;
    Logger::debug(LogCategory::UI, QStringLiteral("Section 6 cards: %1").arg(categoryStreamsCards.size()));
    categoryStreamsSection["cards"] = categoryStreamsCards;
    sections[5] = categoryStreamsSection;
  } else {
    Logger::warning(LogCategory::UI, QStringLiteral("Sections size is %1, cannot update section 6").arg(sections.size()));
  }
  
  m_sectionsData = sections;
  emit sectionsDataChanged();
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
  m_followedStreams = transformTwitchStreams(twitchStreams);
  emit followedStreamsChanged();
  updateSectionsData();
}

void HomeViewModel::updateRecommendedStreams(const QVariantList& twitchStreams) {
  m_recommendedStreams = transformTwitchStreams(twitchStreams);
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
  m_popularClips = transformClips(twitchClips);
  Logger::debug(LogCategory::UI, QStringLiteral("Popular clips updated: %1").arg(m_popularClips.size()));
  emit popularClipsChanged();
  updateSectionsData();
}

void HomeViewModel::updateFollowedClips(const QVariantList& twitchClips) {
  m_followedClips = transformClips(twitchClips);
  Logger::debug(LogCategory::UI, QStringLiteral("Followed clips updated: %1").arg(m_followedClips.size()));
  emit followedClipsChanged();
  updateSectionsData();
}

void HomeViewModel::updateVideos(const QVariantList& twitchVideos) {
  m_videos = transformVideos(twitchVideos);
  Logger::debug(LogCategory::UI, QStringLiteral("Videos updated: %1").arg(m_videos.size()));
  emit videosChanged();
  updateSectionsData();
}

void HomeViewModel::updateFollowedChannels(const QVariantList& twitchChannels) {
  m_followedChannels = transformChannels(twitchChannels);
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
    transformedStreamer[QStringLiteral("title")] = QStringLiteral("Nouveau streamer suivi");
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
  m_categoryStreams = transformTwitchStreams(twitchStreams);
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
  
  // Section 1: Streams suivis (sera remplie dynamiquement)
  QVariantMap section1;
  section1[QStringLiteral("title")] = QStringLiteral("Vos streamers suivis");
  section1[QStringLiteral("subtitle")] = QStringLiteral("Chaînes en direct");
  section1[QStringLiteral("cards")] = m_placeholderCards;  // Toujours utiliser les placeholders par défaut, sera remplacé dans updateSectionsData()
  sections.append(section1);
  
  // Section 2: Recommandé pour vous (utilisera m_recommendedStreams)
  QVariantMap section2;
  section2[QStringLiteral("title")] = QStringLiteral("Recommandé pour vous");
  section2[QStringLiteral("subtitle")] = QStringLiteral("Basé sur vos préférences");
  section2[QStringLiteral("type")] = QStringLiteral("streams");
  section2[QStringLiteral("cards")] = m_placeholderCards;  // Sera remplacé par de vraies données dans updateSectionsData()
  sections.append(section2);
  
  // Section 3: Parcourir (catégories)
  QVariantMap section3;
  section3[QStringLiteral("title")] = QStringLiteral("Parcourir");
  section3[QStringLiteral("subtitle")] = QStringLiteral("Découvrez les catégories populaires");
  QVariantList categoryCards;
  // Cartes par défaut si aucune catégorie n'est chargée
  categoryCards.append(createCategoryCard("Just Chatting", "509658", ""));
  categoryCards.append(createCategoryCard("League of Legends", "21779", ""));
  categoryCards.append(createCategoryCard("Fortnite", "33214", ""));
  categoryCards.append(createCategoryCard("VALORANT", "516575", ""));
  categoryCards.append(createCategoryCard("Minecraft", "27471", ""));
  categoryCards.append(createCategoryCard("GTA V", "32982", ""));
  section3[QStringLiteral("cards")] = categoryCards;
  sections.append(section3);
  
  // Section 4: En direct maintenant (utilisera m_recommendedStreams)
  QVariantMap section4;
  section4[QStringLiteral("title")] = QStringLiteral("En direct maintenant");
  section4[QStringLiteral("subtitle")] = QStringLiteral("Les streams les plus populaires");
  section4[QStringLiteral("type")] = QStringLiteral("streams");
  section4[QStringLiteral("cards")] = m_placeholderCards;  // Sera remplacé par de vraies données dans updateSectionsData()
  sections.append(section4);
  
  // Section 5: Clips populaires
  QVariantMap section5;
  section5[QStringLiteral("title")] = QStringLiteral("Clips populaires");
  section5[QStringLiteral("subtitle")] = QStringLiteral("Les meilleurs moments");
  section5[QStringLiteral("type")] = QStringLiteral("clips");
  section5[QStringLiteral("cards")] = m_placeholderCards;
  sections.append(section5);
  
  // Section 6: Recommandations par catégorie (dynamique)
  QVariantMap section6;
  section6[QStringLiteral("title")] = QStringLiteral("Recommandations par catégorie");
  section6[QStringLiteral("subtitle")] = QStringLiteral("Découvrez par jeu");
  section6[QStringLiteral("type")] = QStringLiteral("streams");
  section6[QStringLiteral("cards")] = m_placeholderCards;
  sections.append(section6);
  
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

}  // namespace blueplayer::ui

