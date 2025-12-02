#pragma once

#include <QObject>
#include <QNetworkDiskCache>
#include <QString>

namespace blueplayer::core {

/**
 * @brief Gestionnaire de cache réseau pour les requêtes API
 * 
 * Utilise QNetworkDiskCache pour mettre en cache les réponses réseau
 * avec un TTL configurable.
 */
class NetworkCache : public QNetworkDiskCache {
  Q_OBJECT

public:
  explicit NetworkCache(QObject* parent = nullptr);

  /**
   * @brief Configure le cache avec la taille et le TTL
   * @param cacheSize Taille maximale du cache en octets
   * @param cacheTTL TTL en secondes
   */
  void configure(int cacheSize, int cacheTTL);

  /**
   * @brief Obtient le TTL configuré
   * @return Le TTL en secondes
   */
  [[nodiscard]] int cacheTTL() const { return m_cacheTTL; }

protected:
  /**
   * @brief Vérifie si une entrée de cache est encore valide
   * @param metaData Les métadonnées de l'entrée
   * @return true si l'entrée est valide
   */
  [[nodiscard]] bool isEntryValid(const QNetworkCacheMetaData& metaData) const;

private:
  int m_cacheTTL = 300;  // 5 minutes par défaut
};

}  // namespace blueplayer::core

