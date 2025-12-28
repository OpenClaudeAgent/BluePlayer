#pragma once

#include <QObject>
#include <memory>

// Forward declarations pour éviter les dépendances circulaires

namespace blueplayer::api::twitch {
class TwitchService;
}

namespace blueplayer::core {

class CacheManager;
class ISecureStorage;

class Application final : public QObject {
  Q_OBJECT

 public:
  /**
   * @brief Constructeur avec injection de dépendances optionnelle
   * @param secureStorage Stockage sécurisé à utiliser (nullptr = Keychain macOS)
   * @param parent Le parent QObject
   */
  explicit Application(ISecureStorage* secureStorage = nullptr, QObject* parent = nullptr);
  ~Application();  // Déclaré ici, défini dans .cpp pour permettre forward
                   // declarations

  // Point d'extension futur pour initialiser les services (API, streaming,
  // etc.)
  void initialize();

  [[nodiscard]] blueplayer::api::twitch::TwitchService* twitchService() const;
  [[nodiscard]] CacheManager* cacheManager() const;

 private:
  std::unique_ptr<blueplayer::api::twitch::TwitchService> m_twitchService;
  std::unique_ptr<CacheManager> m_cacheManager;
};

}  // namespace blueplayer::core
