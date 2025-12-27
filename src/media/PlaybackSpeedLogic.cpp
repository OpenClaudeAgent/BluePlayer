#include "media/PlaybackSpeedLogic.hpp"

#include <QObject>
#include <QtGlobal>
#include <cmath>

namespace blueplayer::media {

double PlaybackSpeedLogic::liveEdgeDelta(const State& s) {
    if (s.duration <= 0.0) {
        return 0.0;
    }
    return s.duration - s.position;
}

bool PlaybackSpeedLogic::isNearLiveEdge(const State& s) {
    if (s.duration <= 0.0) {
        return false;
    }
    return liveEdgeDelta(s) < LIVE_EDGE_THRESHOLD;
}

bool PlaybackSpeedLogic::isApproachingLiveEdge(const State& s) {
    if (s.duration <= 0.0) {
        return false;
    }
    return liveEdgeDelta(s) < APPROACHING_LIVE_THRESHOLD;
}

double PlaybackSpeedLogic::clampRate(double rate) {
    return qBound(MIN_RATE, rate, MAX_RATE);
}

PlaybackSpeedLogic::RateChangeResult 
PlaybackSpeedLogic::computeRateChange(const State& s, double requestedRate) {
    RateChangeResult result{};
    result.finalRate = clampRate(requestedRate);
    result.liveModeChanged = false;
    result.newLiveMode = s.isLiveMode;
    result.wasLimited = false;
    result.wasAutoReset = false;

    // If in live mode, limit max speed to prevent stuttering
    if (s.isLiveMode && result.finalRate > MAX_SPEED_AT_LIVE) {
        result.finalRate = MAX_SPEED_AT_LIVE;
        result.wasLimited = true;
        result.message = QObject::tr("Vitesse limitée (déjà au live)");
    }

    // If slowing down while in live mode, we'll fall behind - exit live mode
    if (s.isLiveMode && result.finalRate < DEFAULT_RATE) {
        result.liveModeChanged = true;
        result.newLiveMode = false;
    }

    return result;
}

PlaybackSpeedLogic::AutoResetResult 
PlaybackSpeedLogic::checkAutoReset(const State& s) {
    AutoResetResult result{};
    result.shouldReset = false;

    // Only check if playing faster than normal
    if (s.currentRate <= DEFAULT_RATE) {
        return result;
    }

    // Need valid duration
    if (s.duration <= 0.0) {
        return result;
    }

    // Threshold is proportional to playback rate to avoid buffer underrun
    // At 3x, we consume buffer 3x faster, so need 3x more margin
    double speedResetThreshold = APPROACHING_LIVE_THRESHOLD * s.currentRate;
    double delta = liveEdgeDelta(s);

    if (delta < speedResetThreshold) {
        result.shouldReset = true;
        result.message = QObject::tr("Vitesse réinitialisée (live)");
    }

    return result;
}

bool PlaybackSpeedLogic::isSignificantChange(double oldRate, double newRate) {
    return !qFuzzyCompare(oldRate, newRate);
}

} // namespace blueplayer::media
