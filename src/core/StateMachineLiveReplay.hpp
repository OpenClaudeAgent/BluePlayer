#pragma once

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <atomic>
#include <functional>
#include <queue>

namespace blueplayer::core {

/**
 * @brief États de la state machine Live/Replay
 *
 * Définit tous les états possibles pour la gestion du live et du mode replay.
 */
enum class LiveReplayState {
  /// Lecture en direct, le playhead suit le live edge
  Live = 0,

  /// Préparation au seekback (calcul de la fenêtre, vérification des ressources)
  SeekbackPrep,

  /// L'utilisateur explore le replay (navigation dans le temps)
  SeekbackActive,

  /// Opération de positionnement en cours
  SeekbackSeeking,

  /// Replay en cours, lecture depuis le cache
  ReplayPlaying,

  /// Replay mis en pause
  ReplayPaused,

  /// Transition rapide pour revenir au live
  CatchUpToLive,

  /// État d'erreur (buffer underrun, perte réseau, etc.)
  Error
};

/**
 * @brief Événements déclenchant les transitions de la state machine
 */
enum class LiveReplayEvent {
  /// Demande de seekback par l'utilisateur
  SeekBackRequest,

  /// Demande de seek vers un temps spécifique
  SeekToTime,

  /// Lancer la lecture depuis le replay
  PlayFromReplay,

  /// Mettre en pause le replay
  PauseFromReplay,

  /// Revenir au live
  JumpToLive,

  /// Le live edge a avancé
  LiveEdgeAdvanced,

  /// Buffer insuffisant
  BufferUnderrun,

  /// Buffer rechargé
  BufferRefill,

  /// Fin du replay (début du cache atteint)
  EndOfReplay,

  /// Erreur survenue
  ErrorOccurred,

  /// Opération de seek terminée
  SeekCompleted,

  /// Mise à jour de la durée du cache
  CacheUpdated
};

/**
 * @brief Structure représentant un événement avec ses données associées
 */
struct LiveReplayEventData {
  LiveReplayEvent event;
  double targetTime = 0.0;     ///< Temps cible pour SeekToTime
  double cacheDuration = 0.0;  ///< Nouvelle durée du cache pour CacheUpdated
  QString errorMessage;        ///< Message d'erreur pour ErrorOccurred
};

/**
 * @brief State machine performante pour la gestion Live/Replay
 *
 * Gère les transitions entre les modes live et replay avec:
 * - Fenêtre de seekback dynamique basée sur la durée du cache
 * - File d'événements lock-free pour les performances
 * - Worker thread dédié pour éviter le blocage de l'UI
 * - Signaux Qt pour l'intégration QML
 */
class StateMachineLiveReplay : public QObject {
  Q_OBJECT

  // Propriétés exposées à QML
  Q_PROPERTY(int state READ stateInt NOTIFY stateChanged)
  Q_PROPERTY(double currentTime READ currentTime NOTIFY currentTimeChanged)
  Q_PROPERTY(double liveEdgeTime READ liveEdgeTime NOTIFY liveEdgeTimeChanged)
  Q_PROPERTY(
      double cacheDuration READ cacheDuration NOTIFY cacheDurationChanged)
  Q_PROPERTY(
      double windowSeekback READ windowSeekback NOTIFY windowSeekbackChanged)
  Q_PROPERTY(bool isInReplay READ isInReplay NOTIFY isInReplayChanged)
  Q_PROPERTY(double bufferSeconds READ bufferSeconds NOTIFY bufferSecondsChanged)
  Q_PROPERTY(double latency READ latency NOTIFY latencyChanged)

 public:
  /**
   * @brief Constructeur
   * @param parent QObject parent
   */
  explicit StateMachineLiveReplay(QObject* parent = nullptr);

  /**
   * @brief Destructeur - arrête le worker thread proprement
   */
  ~StateMachineLiveReplay() override;

  // ===== Getters pour les propriétés =====

  /**
   * @brief Obtient l'état actuel
   * @return L'état actuel de la state machine
   */
  [[nodiscard]] LiveReplayState state() const {
    return m_state.load(std::memory_order_acquire);
  }

  /**
   * @brief Obtient l'état en tant qu'entier (pour QML)
   * @return L'état en tant qu'entier
   */
  [[nodiscard]] int stateInt() const { return static_cast<int>(state()); }

  /**
   * @brief Obtient le temps de lecture actuel
   * @return Le temps actuel en secondes
   */
  [[nodiscard]] double currentTime() const {
    return m_currentTime.load(std::memory_order_acquire);
  }

  /**
   * @brief Obtient le temps du live edge
   * @return Le temps du bord live en secondes
   */
  [[nodiscard]] double liveEdgeTime() const {
    return m_liveEdgeTime.load(std::memory_order_acquire);
  }

  /**
   * @brief Obtient la durée totale du cache
   * @return La durée du cache en secondes
   */
  [[nodiscard]] double cacheDuration() const {
    return m_cacheDuration.load(std::memory_order_acquire);
  }

  /**
   * @brief Obtient la fenêtre de seekback disponible
   * @return La fenêtre de seekback (= cacheDuration) en secondes
   */
  [[nodiscard]] double windowSeekback() const {
    return m_cacheDuration.load(std::memory_order_acquire);
  }

  /**
   * @brief Vérifie si on est en mode replay
   * @return true si en mode replay, false si en live
   */
  [[nodiscard]] bool isInReplay() const;

  /**
   * @brief Obtient le niveau de buffer actuel
   * @return Les secondes de buffer disponibles
   */
  [[nodiscard]] double bufferSeconds() const {
    return m_bufferSeconds.load(std::memory_order_acquire);
  }

  /**
   * @brief Obtient la latence actuelle (liveEdge - currentTime)
   * @return La latence en secondes
   */
  [[nodiscard]] double latency() const;

  /**
   * @brief Obtient le nom de l'état actuel (pour debug/logs)
   * @param state L'état à convertir
   * @return Le nom de l'état
   */
  [[nodiscard]] static QString stateName(LiveReplayState state);

  /**
   * @brief Obtient le nom de l'événement (pour debug/logs)
   * @param event L'événement à convertir
   * @return Le nom de l'événement
   */
  [[nodiscard]] static QString eventName(LiveReplayEvent event);

 public slots:
  // ===== Actions utilisateur =====

  /**
   * @brief Démarre une opération de seekback
   *
   * Calcule le temps cible basé sur la fenêtre de seekback et initie
   * la transition vers le mode replay.
   */
  void seekBack();

  /**
   * @brief Seek vers un temps spécifique
   * @param time Le temps cible en secondes
   *
   * Le temps sera clampé entre 0 et liveEdgeTime.
   */
  void seekToTime(double time);

  /**
   * @brief Lance la lecture depuis le replay
   */
  void playFromReplay();

  /**
   * @brief Met en pause le replay
   */
  void pauseFromReplay();

  /**
   * @brief Revient au live
   *
   * Initie une transition vers le mode live, potentiellement
   * via un état CatchUpToLive si nécessaire.
   */
  void jumpToLive();

  // ===== Mises à jour depuis le pipeline média =====

  /**
   * @brief Met à jour le temps de lecture actuel
   * @param time Le nouveau temps de lecture
   */
  void updateCurrentTime(double time);

  /**
   * @brief Met à jour le temps du live edge
   * @param time Le nouveau temps du live edge
   */
  void updateLiveEdgeTime(double time);

  /**
   * @brief Met à jour la durée du cache
   * @param duration La nouvelle durée du cache en secondes
   */
  void updateCacheDuration(double duration);

  /**
   * @brief Met à jour le niveau de buffer
   * @param seconds Les secondes de buffer disponibles
   */
  void updateBufferSeconds(double seconds);

  /**
   * @brief Signale un buffer underrun
   */
  void notifyBufferUnderrun();

  /**
   * @brief Signale que le buffer est rechargé
   */
  void notifyBufferRefill();

  /**
   * @brief Signale la fin du seek
   */
  void notifySeekCompleted();

  /**
   * @brief Signale une erreur
   * @param message Le message d'erreur
   */
  void notifyError(const QString& message);

  /**
   * @brief Réinitialise la state machine à l'état Live
   */
  void reset();

 signals:
  // ===== Signaux de changement d'état =====
  void stateChanged(int newState);
  void currentTimeChanged(double time);
  void liveEdgeTimeChanged(double time);
  void cacheDurationChanged(double duration);
  void windowSeekbackChanged(double window);
  void isInReplayChanged(bool inReplay);
  void bufferSecondsChanged(double seconds);
  void latencyChanged(double latency);

  // ===== Signaux d'action pour le pipeline média =====

  /**
   * @brief Émis quand le pipeline doit effectuer un seek
   * @param targetTime Le temps cible du seek
   */
  void seekRequested(double targetTime);

  /**
   * @brief Émis quand le pipeline doit passer en mode replay
   */
  void replayModeRequested();

  /**
   * @brief Émis quand le pipeline doit revenir en mode live
   */
  void liveModeRequested();

  /**
   * @brief Émis quand le pipeline doit mettre en pause
   */
  void pauseRequested();

  /**
   * @brief Émis quand le pipeline doit reprendre la lecture
   */
  void playRequested();

  /**
   * @brief Émis quand une erreur survient
   * @param message Le message d'erreur
   */
  void errorOccurred(const QString& message);

  /**
   * @brief Émis pour le logging/debug des transitions
   * @param fromState L'état de départ
   * @param toState L'état d'arrivée
   * @param event L'événement déclencheur
   */
  void transitionOccurred(int fromState, int toState, int event);

 private:
  /**
   * @brief Enfile un événement pour traitement
   * @param eventData Les données de l'événement
   */
  void enqueueEvent(const LiveReplayEventData& eventData);

  /**
   * @brief Traite un événement et effectue la transition si applicable
   * @param eventData Les données de l'événement
   */
  void processEvent(const LiveReplayEventData& eventData);

  /**
   * @brief Effectue une transition d'état
   * @param newState Le nouvel état
   * @param event L'événement déclencheur
   */
  void transitionTo(LiveReplayState newState, LiveReplayEvent event);

  /**
   * @brief Vérifie si une transition est valide
   * @param from L'état de départ
   * @param to L'état d'arrivée
   * @param event L'événement
   * @return true si la transition est valide
   */
  [[nodiscard]] bool isValidTransition(LiveReplayState from,
                                       LiveReplayState to,
                                       LiveReplayEvent event) const;

  /**
   * @brief Boucle du worker thread
   */
  void workerLoop();

  // ===== État atomique =====
  std::atomic<LiveReplayState> m_state{LiveReplayState::Live};
  std::atomic<double> m_currentTime{0.0};
  std::atomic<double> m_liveEdgeTime{0.0};
  std::atomic<double> m_cacheDuration{0.0};
  std::atomic<double> m_bufferSeconds{0.0};

  // ===== File d'événements thread-safe =====
  std::queue<LiveReplayEventData> m_eventQueue;
  QMutex m_queueMutex;
  QWaitCondition m_queueCondition;
  std::atomic<bool> m_hasEvents{false};

  // ===== Worker thread =====
  QThread* m_workerThread = nullptr;
  std::atomic<bool> m_running{false};
  std::atomic<bool> m_useWorkerThread{false};

  // ===== Métriques =====
  double m_lastLatency = 0.0;
  bool m_wasInReplay = false;

 public:
  /**
   * @brief Active le worker thread dédié pour le traitement des événements
   *
   * Quand activé, les événements sont traités sur un thread séparé,
   * libérant le thread principal pour l'UI.
   */
  void enableWorkerThread();

  /**
   * @brief Désactive le worker thread
   *
   * Les événements seront traités sur le thread principal via QTimer.
   */
  void disableWorkerThread();

  /**
   * @brief Vérifie si le worker thread est actif
   * @return true si le worker thread est utilisé
   */
  [[nodiscard]] bool isWorkerThreadEnabled() const {
    return m_useWorkerThread.load(std::memory_order_acquire);
  }
};

}  // namespace blueplayer::core



