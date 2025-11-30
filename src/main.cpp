#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QStringLiteral>

#include "core/Application.hpp"

using blueplayer::core::Application;

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  Application coreApp;
  coreApp.initialize();

  QQmlApplicationEngine engine;
  const QUrl url(QStringLiteral("qrc:/BluePlayer/ui/main.qml")); // Chemin corrigé pour les ressources QML
  QObject::connect(
      &engine,
      &QQmlApplicationEngine::objectCreationFailed,
      &app,
      []() { QCoreApplication::exit(-1); },
      Qt::QueuedConnection);

  engine.load(url);
  return QGuiApplication::exec();
}

