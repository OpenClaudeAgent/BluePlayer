#pragma once

#include <QObject>

namespace blueplayer::core {

class Application final : public QObject {
  Q_OBJECT

public:
  explicit Application(QObject* parent = nullptr);

  // Point d’extension futur pour initialiser les services (API, streaming, etc.)
  void initialize();
};

}  // namespace blueplayer::core

