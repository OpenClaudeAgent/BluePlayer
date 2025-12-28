#include <QApplication>
#include <QByteArray>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QStringLiteral>
#include <QTranslator>
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

#include "chat/TwitchChatClient.hpp"
#include "core/CacheManager.hpp"
#include "core/FileLogger.hpp"
#include "media/MpvQuickItem.hpp"
#include "ui/CacheManagerViewModel.hpp"
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

  // Initialize file logging (logs to logs/blueplayer_YYYY-MM-DD_HH-MM-SS.log)
  blueplayer::core::FileLogger::initialize();

  // Fix for MPV: Ensure standard C locale for number parsing
  // Must be called AFTER QApplication because QApp resets locale to system
  // default
  std::setlocale(LC_NUMERIC, "C");

  app.setApplicationName("BluePlayer");
  app.setOrganizationName("BluePlayer");
  app.setOrganizationDomain("blueplayer.app");
  app.setApplicationDisplayName(QStringLiteral(u"BluePlayer"));

  // Load translations
  QTranslator translator;
  QSettings settings;
  QString preferredLanguage = settings.value("i18n/language", "system").toString();
  QString locale;

  if (preferredLanguage.isEmpty() || preferredLanguage == "system") {
    // Use system locale
    locale = QLocale::system().name(); // e.g., "fr_FR", "en_US"
  } else {
    locale = preferredLanguage;
  }

  // Try to load the translation
  if (translator.load("blueplayer_" + locale, ":/i18n")) {
    app.installTranslator(&translator);
    qDebug() << "[i18n] Loaded translation for:" << locale;
  } else if (locale.startsWith("fr") &&
             translator.load("blueplayer_fr", ":/i18n")) {
    // Fallback for any French locale
    app.installTranslator(&translator);
    qDebug() << "[i18n] Loaded French translation (fallback)";
  } else {
    qDebug() << "[i18n] Using default English (no translation loaded for:"
             << locale << ")";
  }
  // FFmpegMediaService removed

  qmlRegisterType<blueplayer::media::MpvQuickItem>("BluePlayer.Media", 1, 0,
                                                   "MpvQuickItem");
  qmlRegisterType<blueplayer::api::twitch::TwitchService>(
      "BluePlayer.Twitch", 1, 0, "TwitchService");
  qmlRegisterType<blueplayer::ui::HomeViewModel>("BluePlayer.UI", 1, 0,
                                                  "HomeViewModel");
  qmlRegisterType<blueplayer::ui::CacheManagerViewModel>(
      "BluePlayer.UI", 1, 0, "CacheManagerViewModel");
  qmlRegisterType<BluePlayer::TwitchChatClient>(
      "BluePlayer.Chat", 1, 0, "TwitchChatClient");
  Application coreApp;
  coreApp.initialize();

  QQmlApplicationEngine engine;
  // FFmpegMediaService removed

  engine.rootContext()->setContextProperty("twitchService",
                                           coreApp.twitchService());
  engine.rootContext()->setContextProperty("cacheManager",
                                           coreApp.cacheManager());
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

  int result = app.exec();
  blueplayer::core::FileLogger::shutdown();
  return result;
}
