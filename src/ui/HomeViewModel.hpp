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
  Q_PROPERTY(QVariantList popularClips READ popularClips NOTIFY popularClipsChanged)
  Q_PROPERTY(QVariantList followedClips READ followedClips NOTIFY followedClipsChanged)
  Q_PROPERTY(QVariantList videos READ videos NOTIFY videosChanged)
  Q_PROPERTY(QVariantList followedChannels READ followedChannels NOTIFY followedChannelsChanged)
  Q_PROPERTY(QVariantList newStreamers READ newStreamers NOTIFY newStreamersChanged)
  Q_PROPERTY(QVariantList categoryStreams READ categoryStreams NOTIFY categoryStreamsChanged)
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
   * @brief Obtient la liste des clips populaires transformés
   * @return La liste des clips populaires
   */
  [[nodiscard]] QVariantList popularClips() const { return m_popularClips; }

  /**
   * @brief Obtient la liste des clips suivis transformés
   * @return La liste des clips suivis
   */
  [[nodiscard]] QVariantList followedClips() const { return m_followedClips; }

  /**
   * @brief Obtient la liste des VODs transformés
   * @return La liste des VODs
   */
  [[nodiscard]] QVariantList videos() const { return m_videos; }

  /**
   * @brief Obtient la liste des chaînes suivies transformées
   * @return La liste des chaînes suivies
   */
  [[nodiscard]] QVariantList followedChannels() const { return m_followedChannels; }

  /**
   * @brief Obtient la liste des streams tendances transformés
   * @return La liste des streams tendances
   */

  /**
   * @brief Obtient la liste des nouveaux streamers transformés
   * @return La liste des nouveaux streamers
   */
  [[nodiscard]] QVariantList newStreamers() const { return m_newStreamers; }

  /**
   * @brief Obtient la liste des streams par catégorie transformés
   * @return La liste des streams par catégorie
   */
  [[nodiscard]] QVariantList categoryStreams() const { return m_categoryStreams; }

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

  /**
   * @brief Met à jour les clips populaires depuis le service Twitch
   * @param twitchClips Les clips bruts depuis l'API Twitch
   */
  Q_INVOKABLE void updatePopularClips(const QVariantList& twitchClips);

  /**
   * @brief Met à jour les clips suivis depuis le service Twitch
   * @param twitchClips Les clips bruts depuis l'API Twitch
   */
  Q_INVOKABLE void updateFollowedClips(const QVariantList& twitchClips);

  /**
   * @brief Met à jour les VODs depuis le service Twitch
   * @param twitchVideos Les VODs bruts depuis l'API Twitch
   */
  Q_INVOKABLE void updateVideos(const QVariantList& twitchVideos);

  /**
   * @brief Met à jour les chaînes suivies depuis le service Twitch
   * @param twitchChannels Les chaînes brutes depuis l'API Twitch
   */
  Q_INVOKABLE void updateFollowedChannels(const QVariantList& twitchChannels);

  /**
   * @brief Met à jour les streams tendances depuis le service Twitch
   * @param twitchStreams Les streams bruts depuis l'API Twitch
   */

  /**
   * @brief Met à jour les nouveaux streamers depuis le service Twitch
   * @param twitchStreamers Les streamers bruts depuis l'API Twitch
   */
  Q_INVOKABLE void updateNewStreamers(const QVariantList& twitchStreamers);

  /**
   * @brief Met à jour les streams par catégorie depuis le service Twitch
   * @param twitchStreams Les streams bruts depuis l'API Twitch
   */
  Q_INVOKABLE void updateCategoryStreams(const QVariantList& twitchStreams);

signals:
  void followedStreamsChanged();
  void recommendedStreamsChanged();
  void categoriesChanged();
  void popularClipsChanged();
  void followedClipsChanged();
  void videosChanged();
  void followedChannelsChanged();
  void newStreamersChanged();
  void categoryStreamsChanged();
  void sectionsDataChanged();

private:
  QVariantList m_followedStreams;
  QVariantList m_recommendedStreams;
  QVariantList m_categories;
  QVariantList m_popularClips;
  QVariantList m_followedClips;
  QVariantList m_videos;
  QVariantList m_followedChannels;
  QVariantList m_newStreamers;
  QVariantList m_categoryStreams;
  QVariantList m_placeholderCards;
  QVariantList m_sectionsData;
  
  void generatePlaceholderCards();
  void updateSectionsData();
  QVariantList createDefaultSections() const;
  QVariantMap createCard(const QString& name, const QString& detail, const QString& viewers) const;
  QVariantMap createCategoryCard(const QString& name, const QString& id, const QString& boxArtUrl) const;
  QVariantList transformClips(const QVariantList& twitchClips);
  QVariantList transformVideos(const QVariantList& twitchVideos);
  QVariantList transformChannels(const QVariantList& twitchChannels);
  
  /**
   * @brief Compare two lists by a key field to detect changes
   * @param oldList The current list
   * @param newList The new list to compare
   * @param keyField The field name to use for comparison (e.g., "streamerLogin", "id")
   * @return true if lists are equivalent (same items in same order), false otherwise
   */
  static bool areListsEquivalent(const QVariantList& oldList, const QVariantList& newList, const QString& keyField);
};

}  // namespace blueplayer::ui

