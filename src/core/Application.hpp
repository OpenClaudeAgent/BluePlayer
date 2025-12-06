#pragma once

#include <QObject>
#include <memory>

// Forward declarations pour éviter les dépendances circulaires

namespace blueplayer::api::twitch {
class TwitchService;
}

namespace blueplayer::core {

class Application final : public QObject {
  Q_OBJECT

public:
  explicit Application(QObject *parent = nullptr);
  ~Application(); // Déclaré ici, défini dans .cpp pour permettre forward
                  // declarations

  // Point d'extension futur pour initialiser les services (API, streaming,
  // etc.)
  void initialize();

  [[nodiscard]] blueplayer::api::twitch::TwitchService *twitchService() const;

private:
  std::unique_ptr<blueplayer::api::twitch::TwitchService> m_twitchService;
};

} // namespace blueplayer::core
