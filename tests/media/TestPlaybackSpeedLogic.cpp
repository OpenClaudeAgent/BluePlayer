#include <QtTest/QtTest>

#include "media/PlaybackSpeedLogic.hpp"

using namespace blueplayer::media;

class TestPlaybackSpeedLogic : public QObject {
  Q_OBJECT

private slots:
  // ===== Tests des constantes =====
  void testConstants();

  // ===== Tests de liveEdgeDelta() =====
  void testLiveEdgeDelta_normalCase();
  void testLiveEdgeDelta_atLiveEdge();
  void testLiveEdgeDelta_zeroDuration();
  void testLiveEdgeDelta_negativeDuration();
  void testLiveEdgeDelta_positionBeyondDuration();

  // ===== Tests de isNearLiveEdge() =====
  void testIsNearLiveEdge_withinThreshold();
  void testIsNearLiveEdge_exactlyAtThreshold();
  void testIsNearLiveEdge_beyondThreshold();
  void testIsNearLiveEdge_atZero();
  void testIsNearLiveEdge_zeroDuration();

  // ===== Tests de isApproachingLiveEdge() =====
  void testIsApproachingLiveEdge_withinThreshold();
  void testIsApproachingLiveEdge_exactlyAtThreshold();
  void testIsApproachingLiveEdge_beyondThreshold();
  void testIsApproachingLiveEdge_nearButNotApproaching();

  // ===== Tests de clampRate() =====
  void testClampRate_withinRange();
  void testClampRate_belowMinimum();
  void testClampRate_aboveMaximum();
  void testClampRate_exactMinimum();
  void testClampRate_exactMaximum();
  void testClampRate_negativeValue();
  void testClampRate_zeroValue();

  // ===== Tests de computeRateChange() - Mode normal =====
  void testComputeRateChange_normalMode_validRate();
  void testComputeRateChange_normalMode_rateNeedsClamping();

  // ===== Tests de computeRateChange() - Mode live =====
  void testComputeRateChange_liveMode_speedBelowLimit();
  void testComputeRateChange_liveMode_speedAboveLimit();
  void testComputeRateChange_liveMode_speedExactlyAtLimit();
  void testComputeRateChange_liveMode_slowingDown();
  void testComputeRateChange_liveMode_speedAtOne();

  // ===== Tests de checkAutoReset() =====
  void testCheckAutoReset_normalSpeed_noReset();
  void testCheckAutoReset_fastSpeed_farFromLive();
  void testCheckAutoReset_fastSpeed_approachingLive();
  void testCheckAutoReset_fastSpeed_exactThreshold();
  void testCheckAutoReset_fastSpeed_dynamicThreshold();
  void testCheckAutoReset_zeroDuration();

  // ===== Tests de isSignificantChange() =====
  void testIsSignificantChange_sameValue();
  void testIsSignificantChange_differentValue();
  void testIsSignificantChange_verySmallDifference();

  // ===== Data-driven tests pour edge cases =====
  void testClampRate_datadriven_data();
  void testClampRate_datadriven();
  void testLiveEdgeDetection_datadriven_data();
  void testLiveEdgeDetection_datadriven();
};

// ===== Tests des constantes =====

void TestPlaybackSpeedLogic::testConstants() {
  // Verify documented threshold values
  QCOMPARE(PlaybackSpeedLogic::LIVE_EDGE_THRESHOLD, 2.0);
  QCOMPARE(PlaybackSpeedLogic::APPROACHING_LIVE_THRESHOLD, 5.0);
  QCOMPARE(PlaybackSpeedLogic::MAX_SPEED_AT_LIVE, 1.2);
  QCOMPARE(PlaybackSpeedLogic::MIN_RATE, 0.25);
  QCOMPARE(PlaybackSpeedLogic::MAX_RATE, 3.0);
  QCOMPARE(PlaybackSpeedLogic::DEFAULT_RATE, 1.0);
}

// ===== Tests de liveEdgeDelta() =====

void TestPlaybackSpeedLogic::testLiveEdgeDelta_normalCase() {
  PlaybackSpeedLogic::State state{100.0, 90.0, 1.0, false};
  QCOMPARE(PlaybackSpeedLogic::liveEdgeDelta(state), 10.0);
}

void TestPlaybackSpeedLogic::testLiveEdgeDelta_atLiveEdge() {
  PlaybackSpeedLogic::State state{100.0, 100.0, 1.0, false};
  QCOMPARE(PlaybackSpeedLogic::liveEdgeDelta(state), 0.0);
}

void TestPlaybackSpeedLogic::testLiveEdgeDelta_zeroDuration() {
  PlaybackSpeedLogic::State state{0.0, 0.0, 1.0, false};
  QCOMPARE(PlaybackSpeedLogic::liveEdgeDelta(state), 0.0);
}

void TestPlaybackSpeedLogic::testLiveEdgeDelta_negativeDuration() {
  PlaybackSpeedLogic::State state{-1.0, 0.0, 1.0, false};
  QCOMPARE(PlaybackSpeedLogic::liveEdgeDelta(state), 0.0);
}

void TestPlaybackSpeedLogic::testLiveEdgeDelta_positionBeyondDuration() {
  // Edge case: position > duration (should not happen but handle gracefully)
  PlaybackSpeedLogic::State state{100.0, 105.0, 1.0, false};
  QCOMPARE(PlaybackSpeedLogic::liveEdgeDelta(state), -5.0);
}

// ===== Tests de isNearLiveEdge() =====

void TestPlaybackSpeedLogic::testIsNearLiveEdge_withinThreshold() {
  // 1.5s from live edge (< 2s threshold)
  PlaybackSpeedLogic::State state{100.0, 98.5, 1.0, false};
  QVERIFY(PlaybackSpeedLogic::isNearLiveEdge(state));
}

void TestPlaybackSpeedLogic::testIsNearLiveEdge_exactlyAtThreshold() {
  // Exactly at 2s threshold - should NOT be near (< not <=)
  PlaybackSpeedLogic::State state{100.0, 98.0, 1.0, false};
  QVERIFY(!PlaybackSpeedLogic::isNearLiveEdge(state));
}

void TestPlaybackSpeedLogic::testIsNearLiveEdge_beyondThreshold() {
  // 3s from live edge (> 2s threshold)
  PlaybackSpeedLogic::State state{100.0, 97.0, 1.0, false};
  QVERIFY(!PlaybackSpeedLogic::isNearLiveEdge(state));
}

void TestPlaybackSpeedLogic::testIsNearLiveEdge_atZero() {
  // At live edge (0s delta)
  PlaybackSpeedLogic::State state{100.0, 100.0, 1.0, false};
  QVERIFY(PlaybackSpeedLogic::isNearLiveEdge(state));
}

void TestPlaybackSpeedLogic::testIsNearLiveEdge_zeroDuration() {
  // Zero duration: can't determine if near live edge, return false
  PlaybackSpeedLogic::State state{0.0, 0.0, 1.0, false};
  QVERIFY(!PlaybackSpeedLogic::isNearLiveEdge(state));
}

// ===== Tests de isApproachingLiveEdge() =====

void TestPlaybackSpeedLogic::testIsApproachingLiveEdge_withinThreshold() {
  // 4s from live edge (< 5s threshold)
  PlaybackSpeedLogic::State state{100.0, 96.0, 1.0, false};
  QVERIFY(PlaybackSpeedLogic::isApproachingLiveEdge(state));
}

void TestPlaybackSpeedLogic::testIsApproachingLiveEdge_exactlyAtThreshold() {
  // Exactly at 5s threshold - should NOT be approaching (< not <=)
  PlaybackSpeedLogic::State state{100.0, 95.0, 1.0, false};
  QVERIFY(!PlaybackSpeedLogic::isApproachingLiveEdge(state));
}

void TestPlaybackSpeedLogic::testIsApproachingLiveEdge_beyondThreshold() {
  // 10s from live edge (> 5s threshold)
  PlaybackSpeedLogic::State state{100.0, 90.0, 1.0, false};
  QVERIFY(!PlaybackSpeedLogic::isApproachingLiveEdge(state));
}

void TestPlaybackSpeedLogic::testIsApproachingLiveEdge_nearButNotApproaching() {
  // 1s from live edge - is near AND approaching
  PlaybackSpeedLogic::State state{100.0, 99.0, 1.0, false};
  QVERIFY(PlaybackSpeedLogic::isNearLiveEdge(state));
  QVERIFY(PlaybackSpeedLogic::isApproachingLiveEdge(state));
}

// ===== Tests de clampRate() =====

void TestPlaybackSpeedLogic::testClampRate_withinRange() {
  QCOMPARE(PlaybackSpeedLogic::clampRate(1.5), 1.5);
  QCOMPARE(PlaybackSpeedLogic::clampRate(2.0), 2.0);
  QCOMPARE(PlaybackSpeedLogic::clampRate(0.5), 0.5);
}

void TestPlaybackSpeedLogic::testClampRate_belowMinimum() {
  QCOMPARE(PlaybackSpeedLogic::clampRate(0.1), 0.25);
  QCOMPARE(PlaybackSpeedLogic::clampRate(0.24), 0.25);
}

void TestPlaybackSpeedLogic::testClampRate_aboveMaximum() {
  QCOMPARE(PlaybackSpeedLogic::clampRate(3.5), 3.0);
  QCOMPARE(PlaybackSpeedLogic::clampRate(3.01), 3.0);
}

void TestPlaybackSpeedLogic::testClampRate_exactMinimum() {
  QCOMPARE(PlaybackSpeedLogic::clampRate(0.25), 0.25);
}

void TestPlaybackSpeedLogic::testClampRate_exactMaximum() {
  QCOMPARE(PlaybackSpeedLogic::clampRate(3.0), 3.0);
}

void TestPlaybackSpeedLogic::testClampRate_negativeValue() {
  QCOMPARE(PlaybackSpeedLogic::clampRate(-1.0), 0.25);
}

void TestPlaybackSpeedLogic::testClampRate_zeroValue() {
  QCOMPARE(PlaybackSpeedLogic::clampRate(0.0), 0.25);
}

// ===== Tests de computeRateChange() - Mode normal =====

void TestPlaybackSpeedLogic::testComputeRateChange_normalMode_validRate() {
  PlaybackSpeedLogic::State state{100.0, 50.0, 1.0, false};
  auto result = PlaybackSpeedLogic::computeRateChange(state, 2.0);

  QCOMPARE(result.finalRate, 2.0);
  QVERIFY(!result.liveModeChanged);
  QVERIFY(!result.wasLimited);
  QVERIFY(!result.wasAutoReset);
  QVERIFY(result.message.isEmpty());
}

void TestPlaybackSpeedLogic::testComputeRateChange_normalMode_rateNeedsClamping() {
  PlaybackSpeedLogic::State state{100.0, 50.0, 1.0, false};
  auto result = PlaybackSpeedLogic::computeRateChange(state, 5.0);

  QCOMPARE(result.finalRate, 3.0);  // Clamped to MAX_RATE
  QVERIFY(!result.liveModeChanged);
  QVERIFY(!result.wasLimited);
}

// ===== Tests de computeRateChange() - Mode live =====

void TestPlaybackSpeedLogic::testComputeRateChange_liveMode_speedBelowLimit() {
  PlaybackSpeedLogic::State state{100.0, 99.0, 1.0, true};
  auto result = PlaybackSpeedLogic::computeRateChange(state, 1.1);

  QCOMPARE(result.finalRate, 1.1);
  QVERIFY(!result.liveModeChanged);
  QVERIFY(!result.wasLimited);
}

void TestPlaybackSpeedLogic::testComputeRateChange_liveMode_speedAboveLimit() {
  PlaybackSpeedLogic::State state{100.0, 99.0, 1.0, true};
  auto result = PlaybackSpeedLogic::computeRateChange(state, 2.0);

  QCOMPARE(result.finalRate, 1.2);  // Limited to MAX_SPEED_AT_LIVE
  QVERIFY(!result.liveModeChanged);
  QVERIFY(result.wasLimited);
  QVERIFY(!result.message.isEmpty());  // Should have toast message
}

void TestPlaybackSpeedLogic::testComputeRateChange_liveMode_speedExactlyAtLimit() {
  PlaybackSpeedLogic::State state{100.0, 99.0, 1.0, true};
  auto result = PlaybackSpeedLogic::computeRateChange(state, 1.2);

  QCOMPARE(result.finalRate, 1.2);
  QVERIFY(!result.wasLimited);  // 1.2 is not > 1.2, so not limited
}

void TestPlaybackSpeedLogic::testComputeRateChange_liveMode_slowingDown() {
  PlaybackSpeedLogic::State state{100.0, 99.0, 1.0, true};
  auto result = PlaybackSpeedLogic::computeRateChange(state, 0.5);

  QCOMPARE(result.finalRate, 0.5);
  QVERIFY(result.liveModeChanged);
  QVERIFY(!result.newLiveMode);  // Should exit live mode
}

void TestPlaybackSpeedLogic::testComputeRateChange_liveMode_speedAtOne() {
  PlaybackSpeedLogic::State state{100.0, 99.0, 1.0, true};
  auto result = PlaybackSpeedLogic::computeRateChange(state, 1.0);

  QCOMPARE(result.finalRate, 1.0);
  QVERIFY(!result.liveModeChanged);  // 1.0 is not < 1.0
}

// ===== Tests de checkAutoReset() =====

void TestPlaybackSpeedLogic::testCheckAutoReset_normalSpeed_noReset() {
  // At normal speed, no auto-reset needed
  PlaybackSpeedLogic::State state{100.0, 96.0, 1.0, false};
  auto result = PlaybackSpeedLogic::checkAutoReset(state);

  QVERIFY(!result.shouldReset);
}

void TestPlaybackSpeedLogic::testCheckAutoReset_fastSpeed_farFromLive() {
  // At 2x speed, threshold is 10s - 20s from live is safe
  PlaybackSpeedLogic::State state{100.0, 80.0, 2.0, false};
  auto result = PlaybackSpeedLogic::checkAutoReset(state);

  QVERIFY(!result.shouldReset);  // 20s > 10s threshold
}

void TestPlaybackSpeedLogic::testCheckAutoReset_fastSpeed_approachingLive() {
  // At 2x speed, threshold is 10s - 9s from live should reset
  PlaybackSpeedLogic::State state{100.0, 91.0, 2.0, false};
  auto result = PlaybackSpeedLogic::checkAutoReset(state);

  QVERIFY(result.shouldReset);
  QVERIFY(!result.message.isEmpty());
}

void TestPlaybackSpeedLogic::testCheckAutoReset_fastSpeed_exactThreshold() {
  // At 2x speed, threshold is 10s - exactly 10s should NOT reset (< not <=)
  PlaybackSpeedLogic::State state{100.0, 90.0, 2.0, false};
  auto result = PlaybackSpeedLogic::checkAutoReset(state);

  QVERIFY(!result.shouldReset);
}

void TestPlaybackSpeedLogic::testCheckAutoReset_fastSpeed_dynamicThreshold() {
  // Test that threshold scales with speed
  // At 3x: threshold = 15s
  PlaybackSpeedLogic::State state3x{100.0, 86.0, 3.0, false};  // 14s from live
  auto result3x = PlaybackSpeedLogic::checkAutoReset(state3x);
  QVERIFY(result3x.shouldReset);  // 14 < 15

  // At 1.5x: threshold = 7.5s
  PlaybackSpeedLogic::State state15x{100.0, 93.0, 1.5, false};  // 7s from live
  auto result15x = PlaybackSpeedLogic::checkAutoReset(state15x);
  QVERIFY(result15x.shouldReset);  // 7 < 7.5

  // At 1.5x: threshold = 7.5s
  PlaybackSpeedLogic::State state15x_safe{100.0, 92.0, 1.5, false};  // 8s from live
  auto result15x_safe = PlaybackSpeedLogic::checkAutoReset(state15x_safe);
  QVERIFY(!result15x_safe.shouldReset);  // 8 > 7.5
}

void TestPlaybackSpeedLogic::testCheckAutoReset_zeroDuration() {
  PlaybackSpeedLogic::State state{0.0, 0.0, 2.0, false};
  auto result = PlaybackSpeedLogic::checkAutoReset(state);

  // Delta is 0, which is < threshold, but also not > 0
  // Should not reset because delta <= 0
  QVERIFY(!result.shouldReset);
}

// ===== Tests de isSignificantChange() =====

void TestPlaybackSpeedLogic::testIsSignificantChange_sameValue() {
  QVERIFY(!PlaybackSpeedLogic::isSignificantChange(1.0, 1.0));
  QVERIFY(!PlaybackSpeedLogic::isSignificantChange(2.5, 2.5));
}

void TestPlaybackSpeedLogic::testIsSignificantChange_differentValue() {
  QVERIFY(PlaybackSpeedLogic::isSignificantChange(1.0, 2.0));
  QVERIFY(PlaybackSpeedLogic::isSignificantChange(1.0, 1.1));
}

void TestPlaybackSpeedLogic::testIsSignificantChange_verySmallDifference() {
  // qFuzzyCompare uses relative epsilon, so very small differences
  // at scale 1.0 are still detected. This tests the actual behavior.
  // The function correctly identifies even tiny differences.
  QVERIFY(PlaybackSpeedLogic::isSignificantChange(1.0, 1.0 + 1e-10));
  
  // But truly identical values are not significant
  double x = 1.5;
  QVERIFY(!PlaybackSpeedLogic::isSignificantChange(x, x));
}

// ===== Data-driven tests =====

void TestPlaybackSpeedLogic::testClampRate_datadriven_data() {
  QTest::addColumn<double>("input");
  QTest::addColumn<double>("expected");

  QTest::newRow("below_min") << 0.1 << 0.25;
  QTest::newRow("exact_min") << 0.25 << 0.25;
  QTest::newRow("low_valid") << 0.5 << 0.5;
  QTest::newRow("normal") << 1.0 << 1.0;
  QTest::newRow("high_valid") << 2.5 << 2.5;
  QTest::newRow("exact_max") << 3.0 << 3.0;
  QTest::newRow("above_max") << 4.0 << 3.0;
  QTest::newRow("negative") << -0.5 << 0.25;
  QTest::newRow("zero") << 0.0 << 0.25;
}

void TestPlaybackSpeedLogic::testClampRate_datadriven() {
  QFETCH(double, input);
  QFETCH(double, expected);

  QCOMPARE(PlaybackSpeedLogic::clampRate(input), expected);
}

void TestPlaybackSpeedLogic::testLiveEdgeDetection_datadriven_data() {
  QTest::addColumn<double>("duration");
  QTest::addColumn<double>("position");
  QTest::addColumn<bool>("expectedNear");
  QTest::addColumn<bool>("expectedApproaching");

  // Near: < 2s, Approaching: < 5s
  QTest::newRow("at_live") << 100.0 << 100.0 << true << true;
  QTest::newRow("1s_behind") << 100.0 << 99.0 << true << true;
  QTest::newRow("1.9s_behind") << 100.0 << 98.1 << true << true;
  QTest::newRow("2s_behind") << 100.0 << 98.0 << false << true;
  QTest::newRow("3s_behind") << 100.0 << 97.0 << false << true;
  QTest::newRow("4.9s_behind") << 100.0 << 95.1 << false << true;
  QTest::newRow("5s_behind") << 100.0 << 95.0 << false << false;
  QTest::newRow("10s_behind") << 100.0 << 90.0 << false << false;
}

void TestPlaybackSpeedLogic::testLiveEdgeDetection_datadriven() {
  QFETCH(double, duration);
  QFETCH(double, position);
  QFETCH(bool, expectedNear);
  QFETCH(bool, expectedApproaching);

  PlaybackSpeedLogic::State state{duration, position, 1.0, false};

  QCOMPARE(PlaybackSpeedLogic::isNearLiveEdge(state), expectedNear);
  QCOMPARE(PlaybackSpeedLogic::isApproachingLiveEdge(state), expectedApproaching);
}

QTEST_MAIN(TestPlaybackSpeedLogic)
#include "TestPlaybackSpeedLogic.moc"
