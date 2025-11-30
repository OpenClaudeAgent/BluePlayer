#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QStringLiteral>

#include "core/Application.hpp"
#include "media/FFmpegMediaService.hpp"

using blueplayer::core::Application;

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  qmlRegisterType<blueplayer::media::FFmpegMediaService>("BluePlayer.Media", 1, 0, "FFmpegMediaService");
  Application coreApp;
  coreApp.initialize();

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("ffmpegService", coreApp.mediaService());
  const QUrl url(QStringLiteral("qrc:/qt/qml/BluePlayer/ui/main.qml"));
  QObject::connect(
      &engine,
      &QQmlApplicationEngine::objectCreationFailed,
      &app,
      []() { QCoreApplication::exit(-1); },
      Qt::QueuedConnection);

  engine.load(url);
  return QGuiApplication::exec();
}

