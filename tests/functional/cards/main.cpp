/**
 * @file main.cpp
 * @brief Entry point for Qt Quick functional UI tests - Cards
 *
 * Tests for card-based components: BaseCard, BlueCard, CategoryCard,
 * ChannelCard, ClipCard, LocalPlaybackCard, StreamCard, VideoCard
 */

#include <QtQuickTest>
#include <QQmlEngine>
#include <QDir>
#include <QDebug>

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() = default;

public slots:
    void qmlEngineAvailable(QQmlEngine* engine)
    {
        QString sourceDir = QStringLiteral(QUICK_TEST_SOURCE_DIR);
        QString projectRoot = QDir(sourceDir).absolutePath() + "/../../..";

        QStringList importPaths = {
            projectRoot + "/src/ui",
            projectRoot + "/src/ui/components",
            projectRoot + "/src/ui/themes"
        };

        for (const QString& path : importPaths) {
            QString absolutePath = QDir(path).absolutePath();
            if (QDir(absolutePath).exists()) {
                engine->addImportPath(absolutePath);
            }
        }
    }

    void cleanupTestCase()
    {
        qDebug() << "Cards functional tests completed";
    }
};

QUICK_TEST_MAIN_WITH_SETUP(FunctionalCards, Setup)

#include "main.moc"
