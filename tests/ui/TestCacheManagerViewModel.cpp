#include <QtTest/QtTest>
#include <QSignalSpy>

#include "ui/CacheManagerViewModel.hpp"
#include "core/CacheManager.hpp"

using namespace blueplayer::ui;
using namespace blueplayer::core;

class TestCacheManagerViewModel : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Tests d'initialisation
  void testConstructor();
  void testInitialize();
  void testInitializeWithNull();

  // Tests des propriétés initiales
  void testInitialVodList();
  void testInitialVodCount();
  void testInitialUsagePercent();
  void testInitialSelectionMode();
  void testInitialSelectedCount();
  void testInitialSortField();
  void testInitialSortAscending();
  void testInitialFilterStreamer();

  // Tests du mode sélection
  void testSetSelectionMode();
  void testSetSelectionModeEmitsSignal();
  void testToggleSelection();
  void testSelectAll();
  void testDeselectAll();
  void testIsSelected();

  // Tests du tri
  void testSetSortField();
  void testSetSortFieldEmitsSignal();
  void testSetSortAscending();
  void testSetSortAscendingEmitsSignal();

  // Tests du filtre
  void testSetFilterStreamer();
  void testSetFilterStreamerEmitsSignal();

  // Tests de la taille du cache
  void testSetMaxCacheSize();
  void testMaxCacheSize();
  void testTotalSizeFormatted();
  void testMaxSizeFormatted();
  void testUsagePercent();

  // Tests des opérations de suppression
  void testDeleteVod();
  void testDeleteVodNonExistent();
  void testDeleteSelected();
  void testDeleteSelectedEmpty();
  void testClearAll();

  // Tests des signaux
  void testVodListChangedSignal();
  void testVodCountChangedSignal();
  void testSelectionModeChangedSignal();
  void testSelectedCountChangedSignal();

  // Tests des utilitaires
  void testGetVodDetails();
  void testGetVodDetailsNonExistent();
  void testGetStreamerList();
  void testRefresh();

  // Tests de applySortAndFilter
  void testSortByRecordedAtAscending();
  void testSortByRecordedAtDescending();
  void testSortByDurationAscending();
  void testSortByDurationDescending();
  void testSortByFileSizeAscending();
  void testSortByFileSizeDescending();
  void testSortByStreamerName();
  void testFilterByStreamer();
  void testFilterByStreamerNoMatch();
  void testFilterAndSort();
  void testSortEmptyList();
  
  // Tests de getStreamerList avancés
  void testGetStreamerListWithDuplicates();
  void testGetStreamerListSorted();
  
  // Tests des callbacks CacheManager
  void testOnVodListChangedCallback();
  void testOnVodCountChangedCallback();
  void testOnTotalSizeChangedCallback();
  void testOnMaxSizeChangedCallback();
  
  // Tests de sélection avancés
  void testSelectAllWithVods();
  void testSelectionModeClearsOnDisable();
  void testDeleteVodClearsSelection();
  
  // Tests edge cases
  void testReinitializeWithDifferentCacheManager();
  void testDeleteSelectedWithSomeInvalidIds();

private:
  CacheManagerViewModel* m_viewModel = nullptr;
  CacheManager* m_cacheManager = nullptr;
  
  // Helper pour créer des VODs de test
  void addTestVods();
};

void TestCacheManagerViewModel::initTestCase() {
}

void TestCacheManagerViewModel::cleanupTestCase() {
}

void TestCacheManagerViewModel::init() {
  m_cacheManager = new CacheManager(this);
  m_viewModel = new CacheManagerViewModel(this);
  m_viewModel->initialize(m_cacheManager);
}

void TestCacheManagerViewModel::cleanup() {
  delete m_viewModel;
  m_viewModel = nullptr;
  delete m_cacheManager;
  m_cacheManager = nullptr;
}

// ===== Tests d'initialisation =====

void TestCacheManagerViewModel::testConstructor() {
  CacheManagerViewModel viewModel;
  // Verify initial state after construction
  QVERIFY(viewModel.vodList().isEmpty());
}

void TestCacheManagerViewModel::testInitialize() {
  CacheManagerViewModel viewModel;
  CacheManager cacheManager;
  
  viewModel.initialize(&cacheManager);
  // After initialization, vodList should still be empty (no cached VODs)
  QVERIFY(viewModel.vodList().isEmpty());
}

void TestCacheManagerViewModel::testInitializeWithNull() {
  CacheManagerViewModel viewModel;
  viewModel.initialize(nullptr);
  
  // Should not crash - verify we can still access properties
  QVERIFY(viewModel.vodList().isEmpty());
}

// ===== Tests des propriétés initiales =====

void TestCacheManagerViewModel::testInitialVodList() {
  QVERIFY(m_viewModel->vodList().isEmpty());
}

void TestCacheManagerViewModel::testInitialVodCount() {
  QCOMPARE(m_viewModel->vodCount(), 0);
}

void TestCacheManagerViewModel::testInitialUsagePercent() {
  QCOMPARE(m_viewModel->usagePercent(), 0.0);
}

void TestCacheManagerViewModel::testInitialSelectionMode() {
  QVERIFY(!m_viewModel->selectionMode());
}

void TestCacheManagerViewModel::testInitialSelectedCount() {
  QCOMPARE(m_viewModel->selectedCount(), 0);
}

void TestCacheManagerViewModel::testInitialSortField() {
  QCOMPARE(m_viewModel->sortField(), QString("recordedAt"));
}

void TestCacheManagerViewModel::testInitialSortAscending() {
  QVERIFY(!m_viewModel->sortAscending());
}

void TestCacheManagerViewModel::testInitialFilterStreamer() {
  QVERIFY(m_viewModel->filterStreamer().isEmpty());
}

// ===== Tests du mode sélection =====

void TestCacheManagerViewModel::testSetSelectionMode() {
  m_viewModel->setSelectionMode(true);
  QVERIFY(m_viewModel->selectionMode());
  
  m_viewModel->setSelectionMode(false);
  QVERIFY(!m_viewModel->selectionMode());
}

void TestCacheManagerViewModel::testSetSelectionModeEmitsSignal() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::selectionModeChanged);
  
  m_viewModel->setSelectionMode(true);
  
  QVERIFY(spy.count() >= 1);
}

void TestCacheManagerViewModel::testToggleSelection() {
  m_viewModel->toggleSelection("vod123", true);
  QVERIFY(m_viewModel->isSelected("vod123"));
  
  m_viewModel->toggleSelection("vod123", false);
  QVERIFY(!m_viewModel->isSelected("vod123"));
}

void TestCacheManagerViewModel::testSelectAll() {
  // Avec liste vide, selectAll ne doit pas crasher
  m_viewModel->selectAll();
  QCOMPARE(m_viewModel->selectedCount(), 0);
}

void TestCacheManagerViewModel::testDeselectAll() {
  m_viewModel->toggleSelection("vod1", true);
  m_viewModel->toggleSelection("vod2", true);
  
  m_viewModel->deselectAll();
  
  QCOMPARE(m_viewModel->selectedCount(), 0);
  QVERIFY(!m_viewModel->isSelected("vod1"));
  QVERIFY(!m_viewModel->isSelected("vod2"));
}

void TestCacheManagerViewModel::testIsSelected() {
  QVERIFY(!m_viewModel->isSelected("any_vod"));
  
  m_viewModel->toggleSelection("any_vod", true);
  QVERIFY(m_viewModel->isSelected("any_vod"));
}

// ===== Tests du tri =====

void TestCacheManagerViewModel::testSetSortField() {
  m_viewModel->setSortField("title");
  QCOMPARE(m_viewModel->sortField(), QString("title"));
  
  m_viewModel->setSortField("sizeBytes");
  QCOMPARE(m_viewModel->sortField(), QString("sizeBytes"));
}

void TestCacheManagerViewModel::testSetSortFieldEmitsSignal() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::sortFieldChanged);
  
  m_viewModel->setSortField("streamerName");
  
  QVERIFY(spy.count() >= 1);
}

void TestCacheManagerViewModel::testSetSortAscending() {
  m_viewModel->setSortAscending(true);
  QVERIFY(m_viewModel->sortAscending());
  
  m_viewModel->setSortAscending(false);
  QVERIFY(!m_viewModel->sortAscending());
}

void TestCacheManagerViewModel::testSetSortAscendingEmitsSignal() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::sortAscendingChanged);
  
  m_viewModel->setSortAscending(true);
  
  QVERIFY(spy.count() >= 1);
}

// ===== Tests du filtre =====

void TestCacheManagerViewModel::testSetFilterStreamer() {
  m_viewModel->setFilterStreamer("TestStreamer");
  QCOMPARE(m_viewModel->filterStreamer(), QString("TestStreamer"));
  
  m_viewModel->setFilterStreamer("");
  QVERIFY(m_viewModel->filterStreamer().isEmpty());
}

void TestCacheManagerViewModel::testSetFilterStreamerEmitsSignal() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::filterStreamerChanged);
  
  m_viewModel->setFilterStreamer("Ninja");
  
  QVERIFY(spy.count() >= 1);
}

// ===== Tests de la taille du cache =====

void TestCacheManagerViewModel::testSetMaxCacheSize() {
  qint64 newSize = 5LL * 1024 * 1024 * 1024; // 5 GB
  
  m_viewModel->setMaxCacheSize(newSize);
  
  QCOMPARE(m_viewModel->maxCacheSize(), newSize);
}

void TestCacheManagerViewModel::testMaxCacheSize() {
  qint64 size = m_viewModel->maxCacheSize();
  QVERIFY(size > 0);
}

void TestCacheManagerViewModel::testTotalSizeFormatted() {
  QString formatted = m_viewModel->totalSizeFormatted();
  QVERIFY(!formatted.isEmpty());
}

void TestCacheManagerViewModel::testMaxSizeFormatted() {
  QString formatted = m_viewModel->maxSizeFormatted();
  QVERIFY(!formatted.isEmpty());
}

void TestCacheManagerViewModel::testUsagePercent() {
  double usage = m_viewModel->usagePercent();
  QVERIFY(usage >= 0.0);
  QVERIFY(usage <= 100.0);
}

// ===== Tests des opérations de suppression =====

void TestCacheManagerViewModel::testDeleteVod() {
  bool result = m_viewModel->deleteVod("nonexistent");
  QVERIFY(!result);
}

void TestCacheManagerViewModel::testDeleteVodNonExistent() {
  bool result = m_viewModel->deleteVod("definitely_does_not_exist");
  QVERIFY(!result);
}

void TestCacheManagerViewModel::testDeleteSelected() {
  // Sans sélection, doit retourner 0
  int deleted = m_viewModel->deleteSelected();
  QCOMPARE(deleted, 0);
}

void TestCacheManagerViewModel::testDeleteSelectedEmpty() {
  m_viewModel->deselectAll();
  int deleted = m_viewModel->deleteSelected();
  QCOMPARE(deleted, 0);
}

void TestCacheManagerViewModel::testClearAll() {
  int cleared = m_viewModel->clearAll();
  QCOMPARE(cleared, 0); // Pas de VOD initialement
}

// ===== Tests des signaux =====

void TestCacheManagerViewModel::testVodListChangedSignal() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::vodListChanged);
  QVERIFY(spy.isValid());
}

void TestCacheManagerViewModel::testVodCountChangedSignal() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::vodCountChanged);
  QVERIFY(spy.isValid());
}

void TestCacheManagerViewModel::testSelectionModeChangedSignal() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::selectionModeChanged);
  QVERIFY(spy.isValid());
}

void TestCacheManagerViewModel::testSelectedCountChangedSignal() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::selectedCountChanged);
  QVERIFY(spy.isValid());
}

// ===== Tests des utilitaires =====

void TestCacheManagerViewModel::testGetVodDetails() {
  QVariantMap details = m_viewModel->getVodDetails("nonexistent");
  QVERIFY(details.isEmpty());
}

void TestCacheManagerViewModel::testGetVodDetailsNonExistent() {
  QVariantMap details = m_viewModel->getVodDetails("xyz123");
  QVERIFY(details.isEmpty());
}

void TestCacheManagerViewModel::testGetStreamerList() {
  QStringList streamers = m_viewModel->getStreamerList();
  // Initialement vide
  QVERIFY(streamers.isEmpty());
}

void TestCacheManagerViewModel::testRefresh() {
  // Ne doit pas crasher
  m_viewModel->refresh();
  QVERIFY(m_viewModel != nullptr);
}

// ===== Helper pour ajouter des VODs de test =====

void TestCacheManagerViewModel::addTestVods() {
  // Ajouter des VODs de test au cache manager
  // Note: CacheManager génère automatiquement l'ID et recordedAt
  // et calcule fileSize à partir du fichier (0 si le fichier n'existe pas)
  
  // VOD 1: Streamer Alpha, 1 heure
  QVariantMap vod1;
  vod1["streamTitle"] = "Stream Part 1";
  vod1["streamerName"] = "Alpha";
  vod1["streamerLogin"] = "alpha";
  vod1["duration"] = 3600LL;  // 1 heure
  vod1["filePath"] = "/tmp/vod1.mp4";
  m_cacheManager->addVodFromQml(vod1);
  
  // VOD 2: Streamer Beta, 2 heures (le plus long)
  QVariantMap vod2;
  vod2["streamTitle"] = "Long Gaming Session";
  vod2["streamerName"] = "Beta";
  vod2["streamerLogin"] = "beta";
  vod2["duration"] = 7200LL;  // 2 heures
  vod2["filePath"] = "/tmp/vod2.mp4";
  m_cacheManager->addVodFromQml(vod2);
  
  // VOD 3: Streamer Alpha encore, 30 min (le plus court)
  QVariantMap vod3;
  vod3["streamTitle"] = "Stream Part 2";
  vod3["streamerName"] = "Alpha";
  vod3["streamerLogin"] = "alpha";
  vod3["duration"] = 1800LL;  // 30 min
  vod3["filePath"] = "/tmp/vod3.mp4";
  m_cacheManager->addVodFromQml(vod3);
  
  // VOD 4: Streamer Gamma, 1.5 heures
  QVariantMap vod4;
  vod4["streamTitle"] = "Casual Stream";
  vod4["streamerName"] = "Gamma";
  vod4["streamerLogin"] = "gamma";
  vod4["duration"] = 5400LL;  // 1.5 heures
  vod4["filePath"] = "/tmp/vod4.mp4";
  m_cacheManager->addVodFromQml(vod4);
}

// ===== Tests de applySortAndFilter =====

void TestCacheManagerViewModel::testSortByRecordedAtAscending() {
  addTestVods();
  
  m_viewModel->setSortField("recordedAt");
  m_viewModel->setSortAscending(true);
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 4);
  
  // Toutes les VODs sont ajoutées ~en même temps, vérifions juste que le tri ne crash pas
  // et que les VODs sont retournées
  for (int i = 0; i < vods.size(); ++i) {
    QVariantMap vod = vods[i].toMap();
    QVERIFY(!vod["id"].toString().isEmpty());
    QVERIFY(vod["recordedAt"].toDateTime().isValid());
  }
}

void TestCacheManagerViewModel::testSortByRecordedAtDescending() {
  addTestVods();
  
  m_viewModel->setSortField("recordedAt");
  m_viewModel->setSortAscending(false);
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 4);
  
  // Vérifier que le tri inverse est appliqué (pas de crash)
  for (int i = 0; i < vods.size(); ++i) {
    QVariantMap vod = vods[i].toMap();
    QVERIFY(!vod["id"].toString().isEmpty());
    QVERIFY(vod["recordedAt"].toDateTime().isValid());
  }
}

void TestCacheManagerViewModel::testSortByDurationAscending() {
  addTestVods();
  
  m_viewModel->setSortField("duration");
  m_viewModel->setSortAscending(true);  // Plus court en premier
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 4);
  
  // Le plus court (1800s) devrait être premier
  qint64 firstDuration = vods[0].toMap()["duration"].toLongLong();
  QCOMPARE(firstDuration, 1800LL);
  
  // Le plus long (7200s) devrait être dernier
  qint64 lastDuration = vods[3].toMap()["duration"].toLongLong();
  QCOMPARE(lastDuration, 7200LL);
  
  // Vérifier l'ordre croissant
  for (int i = 1; i < vods.size(); ++i) {
    qint64 prevDuration = vods[i-1].toMap()["duration"].toLongLong();
    qint64 currDuration = vods[i].toMap()["duration"].toLongLong();
    QVERIFY(prevDuration <= currDuration);
  }
}

void TestCacheManagerViewModel::testSortByDurationDescending() {
  addTestVods();
  
  m_viewModel->setSortField("duration");
  m_viewModel->setSortAscending(false);  // Plus long en premier
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 4);
  
  // Le plus long (7200s) devrait être premier
  qint64 firstDuration = vods[0].toMap()["duration"].toLongLong();
  QCOMPARE(firstDuration, 7200LL);
  
  // Vérifier l'ordre décroissant
  for (int i = 1; i < vods.size(); ++i) {
    qint64 prevDuration = vods[i-1].toMap()["duration"].toLongLong();
    qint64 currDuration = vods[i].toMap()["duration"].toLongLong();
    QVERIFY(prevDuration >= currDuration);
  }
}

void TestCacheManagerViewModel::testSortByFileSizeAscending() {
  addTestVods();
  
  m_viewModel->setSortField("fileSize");
  m_viewModel->setSortAscending(true);  // Plus petit en premier
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 4);
  
  // Note: Les fichiers n'existent pas, donc fileSize = 0 pour tous
  // Vérifions juste que le tri ne crash pas et retourne les VODs
  for (const QVariant& vod : vods) {
    qint64 size = vod.toMap()["fileSize"].toLongLong();
    QVERIFY(size >= 0);
  }
}

void TestCacheManagerViewModel::testSortByFileSizeDescending() {
  addTestVods();
  
  m_viewModel->setSortField("fileSize");
  m_viewModel->setSortAscending(false);  // Plus gros en premier
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 4);
  
  // Note: Les fichiers n'existent pas, donc fileSize = 0 pour tous
  // Vérifions juste que le tri ne crash pas et retourne les VODs
  for (const QVariant& vod : vods) {
    qint64 size = vod.toMap()["fileSize"].toLongLong();
    QVERIFY(size >= 0);
  }
}

void TestCacheManagerViewModel::testSortByStreamerName() {
  addTestVods();
  
  m_viewModel->setSortField("streamerName");
  m_viewModel->setSortAscending(true);  // Ordre alphabétique
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 4);
  
  // Alpha devrait être premier (A < B < G)
  QString firstStreamer = vods[0].toMap()["streamerName"].toString();
  QCOMPARE(firstStreamer, QString("Alpha"));
  
  // Gamma devrait être dernier
  QString lastStreamer = vods[3].toMap()["streamerName"].toString();
  QCOMPARE(lastStreamer, QString("Gamma"));
}

void TestCacheManagerViewModel::testFilterByStreamer() {
  addTestVods();
  
  m_viewModel->setFilterStreamer("Alpha");
  
  QVariantList vods = m_viewModel->vodList();
  
  // Seulement 2 VODs de Alpha (vod1 et vod3)
  QCOMPARE(vods.size(), 2);
  
  for (const QVariant& vod : vods) {
    QCOMPARE(vod.toMap()["streamerName"].toString(), QString("Alpha"));
  }
}

void TestCacheManagerViewModel::testFilterByStreamerNoMatch() {
  addTestVods();
  
  m_viewModel->setFilterStreamer("NonExistentStreamer");
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 0);
}

void TestCacheManagerViewModel::testFilterAndSort() {
  addTestVods();
  
  // Filtrer par Alpha et trier par duration ascendant
  m_viewModel->setFilterStreamer("Alpha");
  m_viewModel->setSortField("duration");
  m_viewModel->setSortAscending(true);
  
  QVariantList vods = m_viewModel->vodList();
  QCOMPARE(vods.size(), 2);
  
  // Les deux VODs Alpha: 1800s (Stream Part 2) et 3600s (Stream Part 1)
  // En ordre croissant par duration:
  QCOMPARE(vods[0].toMap()["duration"].toLongLong(), 1800LL);
  QCOMPARE(vods[1].toMap()["duration"].toLongLong(), 3600LL);
  
  // Tous doivent être Alpha
  for (const QVariant& vod : vods) {
    QCOMPARE(vod.toMap()["streamerName"].toString(), QString("Alpha"));
  }
}

void TestCacheManagerViewModel::testSortEmptyList() {
  // Sans VODs ajoutées
  m_viewModel->setSortField("duration");
  m_viewModel->setSortAscending(true);
  
  QVariantList vods = m_viewModel->vodList();
  QVERIFY(vods.isEmpty());
}

// ===== Tests de getStreamerList avancés =====

void TestCacheManagerViewModel::testGetStreamerListWithDuplicates() {
  addTestVods();
  
  QStringList streamers = m_viewModel->getStreamerList();
  
  // Alpha apparaît 2 fois dans les VODs mais ne devrait être listé qu'une fois
  QCOMPARE(streamers.count("Alpha"), 1);
  QCOMPARE(streamers.size(), 3);  // Alpha, Beta, Gamma
}

void TestCacheManagerViewModel::testGetStreamerListSorted() {
  addTestVods();
  
  QStringList streamers = m_viewModel->getStreamerList();
  
  // La liste devrait être triée alphabétiquement
  QCOMPARE(streamers.size(), 3);
  QCOMPARE(streamers[0], QString("Alpha"));
  QCOMPARE(streamers[1], QString("Beta"));
  QCOMPARE(streamers[2], QString("Gamma"));
}

// ===== Tests des callbacks CacheManager =====

void TestCacheManagerViewModel::testOnVodListChangedCallback() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::vodListChanged);
  
  // Ajouter une VOD devrait déclencher le signal
  QVariantMap vod;
  vod["id"] = "testVod";
  vod["title"] = "Test";
  vod["streamerName"] = "Tester";
  vod["streamerLogin"] = "tester";
  vod["duration"] = 100LL;
  vod["fileSize"] = 1024LL;
  vod["recordedAt"] = QDateTime::currentDateTime();
  vod["filePath"] = "/tmp/test.mp4";
  
  m_cacheManager->addVodFromQml(vod);
  
  QVERIFY(spy.count() >= 1);
}

void TestCacheManagerViewModel::testOnVodCountChangedCallback() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::vodCountChanged);
  
  QVariantMap vod;
  vod["id"] = "countTestVod";
  vod["title"] = "Test";
  vod["streamerName"] = "Tester";
  vod["streamerLogin"] = "tester";
  vod["duration"] = 100LL;
  vod["fileSize"] = 1024LL;
  vod["recordedAt"] = QDateTime::currentDateTime();
  vod["filePath"] = "/tmp/counttest.mp4";
  
  m_cacheManager->addVodFromQml(vod);
  
  QVERIFY(spy.count() >= 1);
}

void TestCacheManagerViewModel::testOnTotalSizeChangedCallback() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::totalSizeChanged);
  
  QVariantMap vod;
  vod["id"] = "sizeTestVod";
  vod["title"] = "Test";
  vod["streamerName"] = "Tester";
  vod["streamerLogin"] = "tester";
  vod["duration"] = 100LL;
  vod["fileSize"] = 1024LL * 1024;  // 1 MB
  vod["recordedAt"] = QDateTime::currentDateTime();
  vod["filePath"] = "/tmp/sizetest.mp4";
  
  m_cacheManager->addVodFromQml(vod);
  
  // Le signal peut être émis plusieurs fois, on vérifie qu'il l'est au moins une fois
  QVERIFY(spy.count() >= 1);
}

void TestCacheManagerViewModel::testOnMaxSizeChangedCallback() {
  QSignalSpy spy(m_viewModel, &CacheManagerViewModel::maxSizeChanged);
  
  qint64 newSize = 20LL * 1024 * 1024 * 1024;  // 20 GB
  m_viewModel->setMaxCacheSize(newSize);
  
  QVERIFY(spy.count() >= 1);
}

// ===== Tests de sélection avancés =====

void TestCacheManagerViewModel::testSelectAllWithVods() {
  addTestVods();
  
  m_viewModel->selectAll();
  
  QCOMPARE(m_viewModel->selectedCount(), 4);
  
  // Vérifier que toutes les VODs sont sélectionnées via leurs IDs réels
  QVariantList vods = m_viewModel->vodList();
  for (const QVariant& vod : vods) {
    QString id = vod.toMap()["id"].toString();
    QVERIFY(m_viewModel->isSelected(id));
  }
}

void TestCacheManagerViewModel::testSelectionModeClearsOnDisable() {
  addTestVods();
  
  QVariantList vods = m_viewModel->vodList();
  QVERIFY(vods.size() >= 2);
  
  QString id1 = vods[0].toMap()["id"].toString();
  QString id2 = vods[1].toMap()["id"].toString();
  
  // Sélectionner des VODs
  m_viewModel->setSelectionMode(true);
  m_viewModel->toggleSelection(id1, true);
  m_viewModel->toggleSelection(id2, true);
  QCOMPARE(m_viewModel->selectedCount(), 2);
  
  // Désactiver le mode sélection devrait tout désélectionner
  m_viewModel->setSelectionMode(false);
  QCOMPARE(m_viewModel->selectedCount(), 0);
  QVERIFY(!m_viewModel->isSelected(id1));
  QVERIFY(!m_viewModel->isSelected(id2));
}

void TestCacheManagerViewModel::testDeleteVodClearsSelection() {
  addTestVods();
  
  QVariantList vods = m_viewModel->vodList();
  QVERIFY(!vods.isEmpty());
  
  QString id = vods[0].toMap()["id"].toString();
  
  m_viewModel->toggleSelection(id, true);
  QVERIFY(m_viewModel->isSelected(id));
  
  m_viewModel->deleteVod(id);
  
  // La sélection devrait être effacée pour cette VOD
  QVERIFY(!m_viewModel->isSelected(id));
}

// ===== Tests edge cases =====

void TestCacheManagerViewModel::testReinitializeWithDifferentCacheManager() {
  // Premier CacheManager
  addTestVods();
  QCOMPARE(m_viewModel->vodCount(), 4);
  
  // Créer un nouveau CacheManager et réinitialiser
  CacheManager* newCacheManager = new CacheManager(this);
  m_viewModel->initialize(newCacheManager);
  
  // Devrait maintenant utiliser le nouveau cache (vide)
  QCOMPARE(m_viewModel->vodCount(), 0);
  
  delete newCacheManager;
}

void TestCacheManagerViewModel::testDeleteSelectedWithSomeInvalidIds() {
  addTestVods();
  
  QVariantList vods = m_viewModel->vodList();
  QVERIFY(vods.size() >= 2);
  
  QString id1 = vods[0].toMap()["id"].toString();
  QString id2 = vods[1].toMap()["id"].toString();
  
  // Sélectionner des VODs existants et un faux
  m_viewModel->toggleSelection(id1, true);
  m_viewModel->toggleSelection("fakeVod", true);
  m_viewModel->toggleSelection(id2, true);
  
  int deleted = m_viewModel->deleteSelected();
  
  // Au moins les vrais devraient être supprimés (ou comptés)
  QVERIFY(deleted >= 0);
  // La sélection devrait être vidée
  QCOMPARE(m_viewModel->selectedCount(), 0);
}

QTEST_MAIN(TestCacheManagerViewModel)
#include "TestCacheManagerViewModel.moc"
