#include <QApplication>
#include <QByteArray>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QStringLiteral>
#include <QWindow>
#include <QtQml>
#include <functional>

#if defined(Q_OS_MAC)
#import <AppKit/AppKit.h>
#include <objc/NSObjCRuntime.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#endif

#include "api/twitch/TwitchService.hpp"
#include "core/Application.hpp"
// #include "media/FFmpegMediaService.hpp" // Removed

#include "media/MpvQuickItem.hpp"
#include "media/MpvFboItem.hpp"
#include "ui/HomeViewModel.hpp"

#if defined(Q_OS_MAC)
@interface PreferencesMenuHandler : NSObject
- (instancetype)initWithCallback:(const std::function<void()> &)callback;
- (void)runPreferences:(id)sender;
@end

@implementation PreferencesMenuHandler {
  std::function<void()> _callback;
}

- (instancetype)initWithCallback:(const std::function<void()> &)callback {
  if (self = [super init]) {
    _callback = callback;
  }
  return self;
}

- (void)runPreferences:(id)sender {
  if (_callback) {
    _callback();
  }
}
@end

static PreferencesMenuHandler *sPreferencesMenuHandler = nil;

void ensureMacPreferencesMenu(const std::function<void()> &callback) {
  if (sPreferencesMenuHandler) {
    [sPreferencesMenuHandler release];
    sPreferencesMenuHandler = nil;
  }
  sPreferencesMenuHandler =
      [[PreferencesMenuHandler alloc] initWithCallback:callback];

  NSMenu *mainMenu = [NSApp mainMenu];
  if (!mainMenu) {
    return;
  }

  NSMenuItem *appMenuItem = [mainMenu itemAtIndex:0];
  NSMenu *appMenu = appMenuItem ? [appMenuItem submenu] : nil;
  if (!appMenu) {
    return;
  }

  NSString *title =
      NSLocalizedString(@"Preferences…", @"BluePlayer preferences menu title");
  for (NSMenuItem *item in [appMenu itemArray]) {
    if ([[item title] isEqualToString:title]) {
      item.target = sPreferencesMenuHandler;
      item.action = @selector(runPreferences:);
      return;
    }
  }

  NSMenuItem *preferencesItem =
      [[NSMenuItem alloc] initWithTitle:title
                                 action:@selector(runPreferences:)
                          keyEquivalent:@","];
  preferencesItem.target = sPreferencesMenuHandler;
  [appMenu insertItem:preferencesItem atIndex:1];
  [preferencesItem release];
}
#endif

#include <clocale>

using blueplayer::core::Application;

int main(int argc, char *argv[]) {
  qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
  QApplication app(argc, argv);

  // Fix for MPV: Ensure standard C locale for number parsing
  // Must be called AFTER QApplication because QApp resets locale to system
  // default
  std::setlocale(LC_NUMERIC, "C");

  app.setApplicationName("BluePlayer");
  app.setApplicationDisplayName(QStringLiteral(u"BluePlayer"));
  // FFmpegMediaService removed

  qmlRegisterType<blueplayer::media::MpvQuickItem>("BluePlayer.Media", 1, 0,
                                                   "MpvQuickItem");
  qmlRegisterType<blueplayer::media::MpvFboItem>("BluePlayer.Media", 1, 0,
                                                 "MpvFboItem");
  qmlRegisterType<blueplayer::api::twitch::TwitchService>(
      "BluePlayer.Twitch", 1, 0, "TwitchService");
  qmlRegisterType<blueplayer::ui::HomeViewModel>("BluePlayer.UI", 1, 0,
                                                 "HomeViewModel");
  Application coreApp;
  coreApp.initialize();

  QQmlApplicationEngine engine;
  // FFmpegMediaService removed

  engine.rootContext()->setContextProperty("twitchService",
                                           coreApp.twitchService());
  const QUrl url(QStringLiteral("qrc:/qt/qml/BluePlayer/ui/main.qml"));
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.load(url);
  auto showPreferences = [&engine]() {
    if (!engine.rootObjects().isEmpty()) {
      QObject *root = engine.rootObjects().first();
      root->setProperty("currentView", "preferences");
    }
  };

#if defined(Q_OS_MAC)
  ensureMacPreferencesMenu(showPreferences);
#endif

  return app.exec();
}
