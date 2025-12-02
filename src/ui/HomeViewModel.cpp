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
  
  // Remplacer la première section avec les streams suivis ou les placeholders
  QVariantMap firstSection = sections.first().toMap();
  QVariantList cards = m_followedStreams.isEmpty() ? m_placeholderCards : m_followedStreams;
  firstSection["cards"] = cards;
  sections[0] = firstSection;
  
  // Remplacer la deuxième section (Recommandé pour vous) avec les streams recommandés réels
  if (sections.size() > 1) {
    QVariantMap recommendedSection = sections[1].toMap();
    QVariantList recommendedCards = m_recommendedStreams.isEmpty() 
      ? recommendedSection["cards"].toList() 
      : m_recommendedStreams;
    recommendedSection["cards"] = recommendedCards;
    sections[1] = recommendedSection;
  }
  
  // Remplacer la troisième section (Parcourir) avec les catégories réelles
  if (sections.size() > 2) {
    QVariantMap browseSection = sections[2].toMap();
    QVariantList categoryCards = m_categories.isEmpty() 
      ? browseSection["cards"].toList() 
      : m_categories;
    browseSection["cards"] = categoryCards;
    sections[2] = browseSection;
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
  
  // Section 2: Recommandé
  QVariantMap section2;
  section2[QStringLiteral("title")] = QStringLiteral("Recommandé pour vous");
  section2[QStringLiteral("subtitle")] = QStringLiteral("Basé sur vos préférences");
  QVariantList recommendedCards;
  recommendedCards.append(createCard("AuroraPlay", "Aventure narrative", "310 viewers"));
  recommendedCards.append(createCard("ZenGarden", "ASMR & mindfulness", "480 viewers"));
  recommendedCards.append(createCard("NeoArena", "Jeux compétitifs", "1 050 viewers"));
  recommendedCards.append(createCard("FluxLuxe", "Talk-show premium", "690 viewers"));
  recommendedCards.append(createCard("PixelCraft", "Création de jeux", "520 viewers"));
  recommendedCards.append(createCard("RetroWave", "Musique rétro", "380 viewers"));
  section2[QStringLiteral("cards")] = recommendedCards;
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
  
  // Section 4: En direct maintenant
  QVariantMap section4;
  section4[QStringLiteral("title")] = QStringLiteral("En direct maintenant");
  section4[QStringLiteral("subtitle")] = QStringLiteral("Les streams les plus populaires");
  QVariantList liveCards;
  liveCards.append(createCard("EpicGamer", "Tournoi esport", "5 240 viewers"));
  liveCards.append(createCard("CreativeHub", "Design & illustration", "3 890 viewers"));
  liveCards.append(createCard("MusicLive", "Concert en direct", "2 670 viewers"));
  liveCards.append(createCard("TechTalk", "Débat technologique", "1 950 viewers"));
  liveCards.append(createCard("FoodieStream", "Cuisine en direct", "1 420 viewers"));
  section4[QStringLiteral("cards")] = liveCards;
  sections.append(section4);
  
  // Section 5: Populaire cette semaine
  QVariantMap section5;
  section5[QStringLiteral("title")] = QStringLiteral("Populaire cette semaine");
  section5[QStringLiteral("subtitle")] = QStringLiteral("Les tendances du moment");
  QVariantList popularCards;
  popularCards.append(createCard("GamingPro", "Speedrun record", "8 500 viewers"));
  popularCards.append(createCard("ArtStudio", "Création en temps réel", "6 200 viewers"));
  popularCards.append(createCard("MusicFest", "Festival virtuel", "4 800 viewers"));
  popularCards.append(createCard("TechReview", "Tests produits", "3 100 viewers"));
  popularCards.append(createCard("CookingShow", "Recettes gourmandes", "2 600 viewers"));
  section5[QStringLiteral("cards")] = popularCards;
  sections.append(section5);
  
  return sections;
}

QVariantMap HomeViewModel::createCard(const QString& name, const QString& detail, const QString& viewers) const {
  QVariantMap card;
  card[QStringLiteral("name")] = name;
  card[QStringLiteral("detail")] = detail;
  card[QStringLiteral("viewers")] = viewers;
  card[QStringLiteral("previewImage")] = QString();  // Pas d'image pour les cartes par défaut
  card[QStringLiteral("isPlaceholder")] = false;  // Ce sont des cartes réelles mais sans image
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

