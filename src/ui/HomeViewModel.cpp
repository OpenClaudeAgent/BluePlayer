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
}

QVariantList HomeViewModel::sectionsData() const {
  QVariantList sections = createDefaultSections();
  
  // Remplacer la première section avec les streams suivis ou les placeholders
  QVariantMap firstSection = sections.first().toMap();
  QVariantList cards = m_followedStreams.isEmpty() ? m_placeholderCards : m_followedStreams;
  firstSection["cards"] = cards;
  sections[0] = firstSection;
  
  // Remplacer la deuxième section (Recommandé pour vous) avec les streams recommandés réels
  // Toujours remplacer la section, même si m_recommendedStreams est vide (pour déclencher le signal)
  if (sections.size() > 1) {
    QVariantMap recommendedSection = sections[1].toMap();
    // Utiliser les données réelles si disponibles, sinon garder les cartes par défaut
    QVariantList recommendedCards = m_recommendedStreams.isEmpty() 
      ? recommendedSection["cards"].toList() 
      : m_recommendedStreams;
    recommendedSection["cards"] = recommendedCards;
    sections[1] = recommendedSection;
  }
  
  return sections;
}

QVariantList HomeViewModel::transformTwitchStreams(const QVariantList& twitchStreams) {
  Logger::debug(LogCategory::UI, QStringLiteral("transformTwitchStreams() called with %1 streams").arg(twitchStreams.size()));
  
  if (twitchStreams.isEmpty()) {
    Logger::debug(LogCategory::UI, QStringLiteral("No streams to transform"));
    return QVariantList();
  }
  
  QVariantList transformed;
  transformed.reserve(twitchStreams.size());
  
  for (const QVariant& streamVar : twitchStreams) {
    const QVariantMap stream = streamVar.toMap();
    QVariantMap transformedStream;
    
    const int viewerCount = stream.value(QStringLiteral("viewer_count")).toInt();
    const QString viewerText = QString::number(viewerCount).replace(QRegularExpression("(\\d)(?=(\\d{3})+(?!\\d))"), "\\1 ") + " viewers";
    
    transformedStream[QStringLiteral("name")] = stream.value(QStringLiteral("user_name")).toString();
    transformedStream[QStringLiteral("detail")] = stream.value(QStringLiteral("title")).toString();
    transformedStream[QStringLiteral("viewers")] = viewerText;
    transformedStream[QStringLiteral("previewImage")] = stream.value(QStringLiteral("thumbnail_url")).toString();
    transformedStream[QStringLiteral("streamUrl")] = stream.value(QStringLiteral("stream_url")).toString();
    
    transformed.append(transformedStream);
  }
  
  Logger::debug(LogCategory::UI, QStringLiteral("Transformed %1 streams").arg(transformed.size()));
  return transformed;
}

void HomeViewModel::updateFollowedStreams(const QVariantList& twitchStreams) {
  m_followedStreams = transformTwitchStreams(twitchStreams);
  emit followedStreamsChanged();
  emit sectionsDataChanged();
}

void HomeViewModel::updateRecommendedStreams(const QVariantList& twitchStreams) {
  Logger::debug(LogCategory::UI, QStringLiteral("updateRecommendedStreams() called with %1 streams").arg(twitchStreams.size()));
  m_recommendedStreams = transformTwitchStreams(twitchStreams);
  Logger::debug(LogCategory::UI, QStringLiteral("Transformed to %1 recommended streams").arg(m_recommendedStreams.size()));
  emit recommendedStreamsChanged();
  emit sectionsDataChanged();
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
  section1[QStringLiteral("cards")] = m_followedStreams.isEmpty() ? m_placeholderCards : m_followedStreams;
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
  
  // Section 3: En direct maintenant
  QVariantMap section3;
  section3[QStringLiteral("title")] = QStringLiteral("En direct maintenant");
  section3[QStringLiteral("subtitle")] = QStringLiteral("Les streams les plus populaires");
  QVariantList liveCards;
  liveCards.append(createCard("EpicGamer", "Tournoi esport", "5 240 viewers"));
  liveCards.append(createCard("CreativeHub", "Design & illustration", "3 890 viewers"));
  liveCards.append(createCard("MusicLive", "Concert en direct", "2 670 viewers"));
  liveCards.append(createCard("TechTalk", "Débat technologique", "1 950 viewers"));
  liveCards.append(createCard("FoodieStream", "Cuisine en direct", "1 420 viewers"));
  section3[QStringLiteral("cards")] = liveCards;
  sections.append(section3);
  
  // Section 4: Populaire cette semaine
  QVariantMap section4;
  section4[QStringLiteral("title")] = QStringLiteral("Populaire cette semaine");
  section4[QStringLiteral("subtitle")] = QStringLiteral("Les tendances du moment");
  QVariantList popularCards;
  popularCards.append(createCard("GamingPro", "Speedrun record", "8 500 viewers"));
  popularCards.append(createCard("ArtStudio", "Création en temps réel", "6 200 viewers"));
  popularCards.append(createCard("MusicFest", "Festival virtuel", "4 800 viewers"));
  popularCards.append(createCard("TechReview", "Tests produits", "3 100 viewers"));
  popularCards.append(createCard("CookingShow", "Recettes gourmandes", "2 600 viewers"));
  section4[QStringLiteral("cards")] = popularCards;
  sections.append(section4);
  
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

}  // namespace blueplayer::ui

