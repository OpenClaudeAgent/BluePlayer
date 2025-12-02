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
   * @brief Obtient les cartes placeholder
   * @return La liste des cartes placeholder
   */
  [[nodiscard]] QVariantList placeholderCards() const { return m_placeholderCards; }

  /**
   * @brief Obtient les données des sections pour l'affichage
   * @return La liste des sections avec leurs données
   */
  [[nodiscard]] QVariantList sectionsData() const;

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

signals:
  void followedStreamsChanged();
  void sectionsDataChanged();

private:
  QVariantList m_followedStreams;
  QVariantList m_placeholderCards;
  
  void generatePlaceholderCards();
  QVariantList createDefaultSections() const;
  QVariantMap createCard(const QString& name, const QString& detail, const QString& viewers) const;
};

}  // namespace blueplayer::ui

