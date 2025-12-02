#pragma once

#include <QObject>
#include <QVariantList>
#include <QString>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace blueplayer::ui {

/**
 * @brief ViewModel pour HomeView
 * 
 * Gère la logique métier et la transformation des données pour l'affichage
 * dans HomeView.qml. Sépare la logique métier de la présentation.
 */
class HomeViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantList followedStreams READ followedStreams NOTIFY followedStreamsChanged)
  Q_PROPERTY(QVariantList recommendedStreams READ recommendedStreams NOTIFY recommendedStreamsChanged)
  Q_PROPERTY(QVariantList categories READ categories NOTIFY categoriesChanged)
  Q_PROPERTY(QVariantList placeholderCards READ placeholderCards CONSTANT)
  Q_PROPERTY(QVariantList sectionsData READ sectionsData NOTIFY sectionsDataChanged)

public:
  explicit HomeViewModel(QObject* parent = nullptr);

  /**
   * @brief Obtient la liste des streams suivis transformés
   * @return La liste des streams
   */
  [[nodiscard]] QVariantList followedStreams() const { return m_followedStreams; }

  /**
   * @brief Obtient la liste des streams recommandés transformés
   * @return La liste des streams recommandés
   */
  [[nodiscard]] QVariantList recommendedStreams() const { return m_recommendedStreams; }

  /**
   * @brief Obtient la liste des catégories transformées
   * @return La liste des catégories
   */
  [[nodiscard]] QVariantList categories() const { return m_categories; }

  /**
   * @brief Obtient les cartes placeholder
   * @return La liste des cartes placeholder
   */
  [[nodiscard]] QVariantList placeholderCards() const { return m_placeholderCards; }

  /**
   * @brief Obtient les données des sections pour l'affichage
   * @return La liste des sections avec leurs données
   */
  [[nodiscard]] QVariantList sectionsData() const { return m_sectionsData; }

  /**
   * @brief Transforme les streams Twitch en format compatible avec StreamCard
   * @param twitchStreams Les streams bruts depuis l'API Twitch
   * @return Les streams transformés
   */
  Q_INVOKABLE QVariantList transformTwitchStreams(const QVariantList& twitchStreams);

  /**
   * @brief Met à jour les streams suivis depuis le service Twitch
   * @param twitchStreams Les streams bruts depuis l'API Twitch
   */
  Q_INVOKABLE void updateFollowedStreams(const QVariantList& twitchStreams);

  /**
   * @brief Met à jour les streams recommandés depuis le service Twitch
   * @param twitchStreams Les streams bruts depuis l'API Twitch
   */
  Q_INVOKABLE void updateRecommendedStreams(const QVariantList& twitchStreams);

  /**
   * @brief Met à jour les catégories depuis le service Twitch
   * @param twitchCategories Les catégories brutes depuis l'API Twitch
   */
  Q_INVOKABLE void updateCategories(const QVariantList& twitchCategories);

signals:
  void followedStreamsChanged();
  void recommendedStreamsChanged();
  void categoriesChanged();
  void sectionsDataChanged();

private:
  QVariantList m_followedStreams;
  QVariantList m_recommendedStreams;
  QVariantList m_categories;
  QVariantList m_placeholderCards;
  QVariantList m_sectionsData;
  
  void generatePlaceholderCards();
  void updateSectionsData();
  QVariantList createDefaultSections() const;
  QVariantMap createCard(const QString& name, const QString& detail, const QString& viewers) const;
  QVariantMap createCategoryCard(const QString& name, const QString& id, const QString& boxArtUrl) const;
};

}  // namespace blueplayer::ui

