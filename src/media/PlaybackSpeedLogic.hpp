#pragma once

#include <QString>

namespace blueplayer::media {

/**
 * Pure business logic for playback speed management.
 * 
 * This class contains no dependencies on mpv or Qt GUI components,
 * making it fully testable in isolation.
 */
class PlaybackSpeedLogic {
public:
    // Speed thresholds for live edge detection
    static constexpr double LIVE_EDGE_THRESHOLD = 2.0;        // < 2s = at live edge
    static constexpr double APPROACHING_LIVE_THRESHOLD = 5.0; // < 5s = approaching
    static constexpr double MAX_SPEED_AT_LIVE = 1.2;          // Max speed allowed at live

    // Playback rate bounds
    static constexpr double MIN_RATE = 0.25;
    static constexpr double MAX_RATE = 3.0;
    static constexpr double DEFAULT_RATE = 1.0;

    /**
     * Input state for speed calculations.
     */
    struct State {
        double duration;     // Total stream duration
        double position;     // Current playback position
        double currentRate;  // Current playback rate
        bool isLiveMode;     // Whether UI is in "live" mode
    };

    /**
     * Result of a rate change computation.
     */
    struct RateChangeResult {
        double finalRate;       // The actual rate to apply
        bool liveModeChanged;   // Whether live mode state changed
        bool newLiveMode;       // New live mode value (if changed)
        bool wasLimited;        // true if rate was limited (for toast)
        bool wasAutoReset;      // true if auto-reset to 1.0
        QString message;        // Message for toast notification (if applicable)
    };

    /**
     * Result of periodic auto-reset check.
     */
    struct AutoResetResult {
        bool shouldReset;   // Whether speed should be reset to 1.0
        QString message;    // Message for toast notification
    };

    // ========================================================================
    // Pure functions - 100% testable without mpv or Qt dependencies
    // ========================================================================

    /**
     * Calculate distance from live edge (duration - position).
     * Returns 0.0 if duration <= 0.
     */
    static double liveEdgeDelta(const State& s);

    /**
     * Check if position is near the live edge (< LIVE_EDGE_THRESHOLD).
     */
    static bool isNearLiveEdge(const State& s);

    /**
     * Check if position is approaching live edge (< APPROACHING_LIVE_THRESHOLD).
     */
    static bool isApproachingLiveEdge(const State& s);

    /**
     * Clamp rate to valid bounds [MIN_RATE, MAX_RATE].
     */
    static double clampRate(double rate);

    /**
     * Compute the result of a rate change request.
     * 
     * This implements the business logic:
     * - Clamp rate to [0.25, 3.0]
     * - Limit to MAX_SPEED_AT_LIVE (1.2) when at live edge
     * - Exit live mode if rate < 1.0 (will fall behind)
     * 
     * @param s Current playback state
     * @param requestedRate The rate requested by the user
     * @return RateChangeResult with the final rate and any state changes
     */
    static RateChangeResult computeRateChange(const State& s, double requestedRate);

    /**
     * Check if speed should be auto-reset when approaching live edge.
     * 
     * When playing faster than 1.0x and approaching live edge,
     * the speed should be reset to avoid buffer underrun.
     * The threshold is proportional to playback rate.
     * 
     * @param s Current playback state
     * @return AutoResetResult indicating if reset is needed
     */
    static AutoResetResult checkAutoReset(const State& s);

    /**
     * Check if a rate change is significant enough to warrant action.
     * Uses fuzzy comparison to avoid floating point issues.
     */
    static bool isSignificantChange(double oldRate, double newRate);
};

} // namespace blueplayer::media
