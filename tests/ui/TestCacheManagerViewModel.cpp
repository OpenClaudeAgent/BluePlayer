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

private:
  CacheManagerViewModel* m_viewModel = nullptr;
  CacheManager* m_cacheManager = nullptr;
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
  QVERIFY(&viewModel != nullptr);
}

void TestCacheManagerViewModel::testInitialize() {
  CacheManagerViewModel viewModel;
  CacheManager cacheManager;
  
  viewModel.initialize(&cacheManager);
  QVERIFY(&viewModel != nullptr);
}

void TestCacheManagerViewModel::testInitializeWithNull() {
  CacheManagerViewModel viewModel;
  viewModel.initialize(nullptr);
  
  // Ne doit pas crasher
  QVERIFY(&viewModel != nullptr);
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

QTEST_MAIN(TestCacheManagerViewModel)
#include "TestCacheManagerViewModel.moc"
