#pragma once

#include <QObject>
#include <QSortFilterProxyModel>
#include <QVariantList>

#include "core/CacheManager.hpp"

namespace blueplayer::ui {

/**
 * @brief ViewModel pour CacheManagerView
 *
 * Gère la logique métier et les opérations sur le cache VOD.
 * Expose les données formatées pour l'affichage dans l'UI QML.
 */
class CacheManagerViewModel : public QObject {
  Q_OBJECT

  Q_PROPERTY(QVariantList vodList READ vodList NOTIFY vodListChanged)
  Q_PROPERTY(int vodCount READ vodCount NOTIFY vodCountChanged)
  Q_PROPERTY(QString totalSizeFormatted READ totalSizeFormatted NOTIFY totalSizeChanged)
  Q_PROPERTY(QString maxSizeFormatted READ maxSizeFormatted NOTIFY maxSizeChanged)
  Q_PROPERTY(double usagePercent READ usagePercent NOTIFY usagePercentChanged)
  Q_PROPERTY(qint64 maxCacheSize READ maxCacheSize WRITE setMaxCacheSize NOTIFY maxSizeChanged)
  Q_PROPERTY(bool selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
  Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectedCountChanged)
  Q_PROPERTY(QString sortField READ sortField WRITE setSortField NOTIFY sortFieldChanged)
  Q_PROPERTY(bool sortAscending READ sortAscending WRITE setSortAscending NOTIFY sortAscendingChanged)
  Q_PROPERTY(QString filterStreamer READ filterStreamer WRITE setFilterStreamer NOTIFY filterStreamerChanged)

 public:
  explicit CacheManagerViewModel(QObject* parent = nullptr);

  /**
   * @brief Initialise le ViewModel avec le CacheManager
   * @param cacheManager Le gestionnaire de cache à utiliser
   */
  Q_INVOKABLE void initialize(blueplayer::core::CacheManager* cacheManager);

  // Getters pour les propriétés
  [[nodiscard]] QVariantList vodList() const;
  [[nodiscard]] int vodCount() const;
  [[nodiscard]] QString totalSizeFormatted() const;
  [[nodiscard]] QString maxSizeFormatted() const;
  [[nodiscard]] double usagePercent() const;
  [[nodiscard]] qint64 maxCacheSize() const;
  [[nodiscard]] bool selectionMode() const { return m_selectionMode; }
  [[nodiscard]] int selectedCount() const { return static_cast<int>(m_selectedIds.size()); }
  [[nodiscard]] QString sortField() const { return m_sortField; }
  [[nodiscard]] bool sortAscending() const { return m_sortAscending; }
  [[nodiscard]] QString filterStreamer() const { return m_filterStreamer; }

  // Setters
  void setMaxCacheSize(qint64 size);
  void setSelectionMode(bool enabled);
  void setSortField(const QString& field);
  void setSortAscending(bool ascending);
  void setFilterStreamer(const QString& streamer);

 public slots:
  /**
   * @brief Sélectionne/désélectionne une VOD
   * @param vodId L'ID de la VOD
   * @param selected true pour sélectionner
   */
  Q_INVOKABLE void toggleSelection(const QString& vodId, bool selected);

  /**
   * @brief Sélectionne toutes les VOD
   */
  Q_INVOKABLE void selectAll();

  /**
   * @brief Désélectionne toutes les VOD
   */
  Q_INVOKABLE void deselectAll();

  /**
   * @brief Vérifie si une VOD est sélectionnée
   * @param vodId L'ID de la VOD
   * @return true si sélectionnée
   */
  Q_INVOKABLE bool isSelected(const QString& vodId) const;

  /**
   * @brief Supprime les VOD sélectionnées
   * @return Nombre de VOD supprimées
   */
  Q_INVOKABLE int deleteSelected();

  /**
   * @brief Supprime une VOD spécifique
   * @param vodId L'ID de la VOD
   * @return true si supprimée avec succès
   */
  Q_INVOKABLE bool deleteVod(const QString& vodId);

  /**
   * @brief Vide tout le cache
   * @return Nombre de VOD supprimées
   */
  Q_INVOKABLE int clearAll();

  /**
   * @brief Obtient les métadonnées d'une VOD
   * @param vodId L'ID de la VOD
   * @return Les métadonnées ou vide si non trouvée
   */
  Q_INVOKABLE QVariantMap getVodDetails(const QString& vodId) const;

  /**
   * @brief Obtient la liste des streamers uniques
   * @return Liste des noms de streamers
   */
  Q_INVOKABLE QStringList getStreamerList() const;

  /**
   * @brief Force le rafraîchissement de la liste
   */
  Q_INVOKABLE void refresh();

 signals:
  void vodListChanged();
  void vodCountChanged();
  void totalSizeChanged();
  void maxSizeChanged();
  void usagePercentChanged();
  void selectionModeChanged();
  void selectedCountChanged();
  void sortFieldChanged();
  void sortAscendingChanged();
  void filterStreamerChanged();

  /**
   * @brief Émis quand une VOD doit être lue
   * @param vodId L'ID de la VOD
   * @param filePath Le chemin du fichier
   */
  void playVodRequested(const QString& vodId, const QString& filePath);

  /**
   * @brief Émis pour afficher une confirmation
   * @param title Titre de la confirmation
   * @param message Message de confirmation
   * @param confirmAction Action à exécuter si confirmé
   */
  void confirmationRequired(const QString& title, const QString& message,
                            const QString& confirmAction);

 private slots:
  void onVodListChanged();
  void onVodCountChanged(int count);
  void onTotalSizeChanged(qint64 size);
  void onMaxSizeChanged(qint64 size);

 private:
  QVariantList applySortAndFilter(const QVariantList& list) const;

  blueplayer::core::CacheManager* m_cacheManager = nullptr;
  QSet<QString> m_selectedIds;
  bool m_selectionMode = false;
  QString m_sortField = QStringLiteral("recordedAt");
  bool m_sortAscending = false;  // Plus récent en premier par défaut
  QString m_filterStreamer;
};

}  // namespace blueplayer::ui
