#include "StateMachineLiveReplay.hpp"

#include <QTimer>

#include "Logger.hpp"

namespace blueplayer::core {

StateMachineLiveReplay::StateMachineLiveReplay(QObject* parent)
    : QObject(parent) {
  // Le worker thread sera démarré à la demande
  LOG_DEBUG(Core, "StateMachineLiveReplay created");
}

StateMachineLiveReplay::~StateMachineLiveReplay() {
  // Arrêter le worker thread proprement
  disableWorkerThread();
  LOG_DEBUG(Core, "StateMachineLiveReplay destroyed");
}

bool StateMachineLiveReplay::isInReplay() const {
  LiveReplayState currentState = m_state.load(std::memory_order_acquire);
  return currentState == LiveReplayState::SeekbackPrep ||
         currentState == LiveReplayState::SeekbackActive ||
         currentState == LiveReplayState::SeekbackSeeking ||
         currentState == LiveReplayState::ReplayPlaying ||
         currentState == LiveReplayState::ReplayPaused;
}

double StateMachineLiveReplay::latency() const {
  double live = m_liveEdgeTime.load(std::memory_order_acquire);
  double current = m_currentTime.load(std::memory_order_acquire);
  return live - current;
}

QString StateMachineLiveReplay::stateName(LiveReplayState state) {
  switch (state) {
    case LiveReplayState::Live:
      return QStringLiteral("Live");
    case LiveReplayState::SeekbackPrep:
      return QStringLiteral("SeekbackPrep");
    case LiveReplayState::SeekbackActive:
      return QStringLiteral("SeekbackActive");
    case LiveReplayState::SeekbackSeeking:
      return QStringLiteral("SeekbackSeeking");
    case LiveReplayState::ReplayPlaying:
      return QStringLiteral("ReplayPlaying");
    case LiveReplayState::ReplayPaused:
      return QStringLiteral("ReplayPaused");
    case LiveReplayState::CatchUpToLive:
      return QStringLiteral("CatchUpToLive");
    case LiveReplayState::Error:
      return QStringLiteral("Error");
  }
  return QStringLiteral("Unknown");
}

QString StateMachineLiveReplay::eventName(LiveReplayEvent event) {
  switch (event) {
    case LiveReplayEvent::SeekBackRequest:
      return QStringLiteral("SeekBackRequest");
    case LiveReplayEvent::SeekToTime:
      return QStringLiteral("SeekToTime");
    case LiveReplayEvent::PlayFromReplay:
      return QStringLiteral("PlayFromReplay");
    case LiveReplayEvent::PauseFromReplay:
      return QStringLiteral("PauseFromReplay");
    case LiveReplayEvent::JumpToLive:
      return QStringLiteral("JumpToLive");
    case LiveReplayEvent::LiveEdgeAdvanced:
      return QStringLiteral("LiveEdgeAdvanced");
    case LiveReplayEvent::BufferUnderrun:
      return QStringLiteral("BufferUnderrun");
    case LiveReplayEvent::BufferRefill:
      return QStringLiteral("BufferRefill");
    case LiveReplayEvent::EndOfReplay:
      return QStringLiteral("EndOfReplay");
    case LiveReplayEvent::ErrorOccurred:
      return QStringLiteral("ErrorOccurred");
    case LiveReplayEvent::SeekCompleted:
      return QStringLiteral("SeekCompleted");
    case LiveReplayEvent::CacheUpdated:
      return QStringLiteral("CacheUpdated");
  }
  return QStringLiteral("Unknown");
}

// ===== Actions utilisateur =====

void StateMachineLiveReplay::seekBack() {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::SeekBackRequest;
  // Calculer le temps cible: début du cache
  double liveEdge = m_liveEdgeTime.load(std::memory_order_acquire);
  double cache = m_cacheDuration.load(std::memory_order_acquire);
  eventData.targetTime = std::max(0.0, liveEdge - cache);
  enqueueEvent(eventData);
}

void StateMachineLiveReplay::seekToTime(double time) {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::SeekToTime;
  // Clamper le temps entre le début du cache et le live edge
  double liveEdge = m_liveEdgeTime.load(std::memory_order_acquire);
  double cache = m_cacheDuration.load(std::memory_order_acquire);
  double minTime = std::max(0.0, liveEdge - cache);
  eventData.targetTime = std::clamp(time, minTime, liveEdge);
  enqueueEvent(eventData);
}

void StateMachineLiveReplay::playFromReplay() {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::PlayFromReplay;
  enqueueEvent(eventData);
}

void StateMachineLiveReplay::pauseFromReplay() {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::PauseFromReplay;
  enqueueEvent(eventData);
}

void StateMachineLiveReplay::jumpToLive() {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::JumpToLive;
  enqueueEvent(eventData);
}

// ===== Mises à jour depuis le pipeline média =====

void StateMachineLiveReplay::updateCurrentTime(double time) {
  double oldTime = m_currentTime.exchange(time, std::memory_order_acq_rel);
  if (std::abs(oldTime - time) > 0.01) {
    emit currentTimeChanged(time);

    // Mettre à jour la latence si changée significativement
    double newLatency = latency();
    if (std::abs(m_lastLatency - newLatency) > 0.1) {
      m_lastLatency = newLatency;
      emit latencyChanged(newLatency);
    }
  }
}

void StateMachineLiveReplay::updateLiveEdgeTime(double time) {
  double oldTime = m_liveEdgeTime.exchange(time, std::memory_order_acq_rel);
  if (std::abs(oldTime - time) > 0.01) {
    emit liveEdgeTimeChanged(time);

    // Émettre un événement LiveEdgeAdvanced pour la state machine
    LiveReplayEventData eventData;
    eventData.event = LiveReplayEvent::LiveEdgeAdvanced;
    enqueueEvent(eventData);
  }
}

void StateMachineLiveReplay::updateCacheDuration(double duration) {
  double oldDuration =
      m_cacheDuration.exchange(duration, std::memory_order_acq_rel);
  if (std::abs(oldDuration - duration) > 0.1) {
    emit cacheDurationChanged(duration);
    emit windowSeekbackChanged(duration);  // windowSeekback = cacheDuration

    LiveReplayEventData eventData;
    eventData.event = LiveReplayEvent::CacheUpdated;
    eventData.cacheDuration = duration;
    enqueueEvent(eventData);
  }
}

void StateMachineLiveReplay::updateBufferSeconds(double seconds) {
  double oldSeconds =
      m_bufferSeconds.exchange(seconds, std::memory_order_acq_rel);
  if (std::abs(oldSeconds - seconds) > 0.1) {
    emit bufferSecondsChanged(seconds);
  }
}

void StateMachineLiveReplay::notifyBufferUnderrun() {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::BufferUnderrun;
  enqueueEvent(eventData);
}

void StateMachineLiveReplay::notifyBufferRefill() {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::BufferRefill;
  enqueueEvent(eventData);
}

void StateMachineLiveReplay::notifySeekCompleted() {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::SeekCompleted;
  enqueueEvent(eventData);
}

void StateMachineLiveReplay::notifyError(const QString& message) {
  LiveReplayEventData eventData;
  eventData.event = LiveReplayEvent::ErrorOccurred;
  eventData.errorMessage = message;
  enqueueEvent(eventData);
}

void StateMachineLiveReplay::reset() {
  // Vider la file d'événements
  {
    QMutexLocker locker(&m_queueMutex);
    std::queue<LiveReplayEventData> empty;
    std::swap(m_eventQueue, empty);
    m_hasEvents.store(false, std::memory_order_release);
  }

  // Réinitialiser les états atomiques
  LiveReplayState oldState = m_state.exchange(LiveReplayState::Live, std::memory_order_acq_rel);
  m_currentTime.store(0.0, std::memory_order_release);
  m_liveEdgeTime.store(0.0, std::memory_order_release);
  m_cacheDuration.store(0.0, std::memory_order_release);
  m_bufferSeconds.store(0.0, std::memory_order_release);
  m_lastLatency = 0.0;

  // Émettre les signaux
  if (oldState != LiveReplayState::Live) {
    emit stateChanged(static_cast<int>(LiveReplayState::Live));
  }
  if (m_wasInReplay) {
    m_wasInReplay = false;
    emit isInReplayChanged(false);
  }

  LOG_INFO(Core, "StateMachineLiveReplay reset to Live state");
}

// ===== Gestion de la file d'événements =====

void StateMachineLiveReplay::enqueueEvent(const LiveReplayEventData& eventData) {
  {
    QMutexLocker locker(&m_queueMutex);
    m_eventQueue.push(eventData);
    m_hasEvents.store(true, std::memory_order_release);

    // Réveiller le worker thread s'il est actif
    if (m_useWorkerThread.load(std::memory_order_acquire)) {
      m_queueCondition.wakeOne();
    }
  }

  // Si le worker thread n'est pas actif, traiter sur le thread principal
  if (!m_useWorkerThread.load(std::memory_order_acquire)) {
    QTimer::singleShot(0, this, [this]() {
      while (m_hasEvents.load(std::memory_order_acquire)) {
        LiveReplayEventData event;
        {
          QMutexLocker locker(&m_queueMutex);
          if (m_eventQueue.empty()) {
            m_hasEvents.store(false, std::memory_order_release);
            break;
          }
          event = m_eventQueue.front();
          m_eventQueue.pop();
          if (m_eventQueue.empty()) {
            m_hasEvents.store(false, std::memory_order_release);
          }
        }
        processEvent(event);
      }
    });
  }
}

void StateMachineLiveReplay::processEvent(const LiveReplayEventData& eventData) {
  LiveReplayState currentState = m_state.load(std::memory_order_acquire);

  LOG_DEBUG(Core, QString("Processing event %1 in state %2")
                      .arg(eventName(eventData.event))
                      .arg(stateName(currentState)));

  switch (eventData.event) {
    case LiveReplayEvent::SeekBackRequest:
    case LiveReplayEvent::SeekToTime: {
      // Depuis Live ou Replay, on peut faire un seek
      if (currentState == LiveReplayState::Live ||
          currentState == LiveReplayState::ReplayPlaying ||
          currentState == LiveReplayState::ReplayPaused ||
          currentState == LiveReplayState::SeekbackActive) {
        transitionTo(LiveReplayState::SeekbackSeeking, eventData.event);
        emit seekRequested(eventData.targetTime);
        if (currentState == LiveReplayState::Live) {
          emit replayModeRequested();
        }
      }
      break;
    }

    case LiveReplayEvent::SeekCompleted: {
      if (currentState == LiveReplayState::SeekbackSeeking) {
        transitionTo(LiveReplayState::ReplayPlaying, eventData.event);
        emit playRequested();
      } else if (currentState == LiveReplayState::CatchUpToLive) {
        transitionTo(LiveReplayState::Live, eventData.event);
        emit liveModeRequested();
      }
      break;
    }

    case LiveReplayEvent::PlayFromReplay: {
      if (currentState == LiveReplayState::ReplayPaused ||
          currentState == LiveReplayState::SeekbackActive) {
        transitionTo(LiveReplayState::ReplayPlaying, eventData.event);
        emit playRequested();
      }
      break;
    }

    case LiveReplayEvent::PauseFromReplay: {
      if (currentState == LiveReplayState::ReplayPlaying) {
        transitionTo(LiveReplayState::ReplayPaused, eventData.event);
        emit pauseRequested();
      }
      break;
    }

    case LiveReplayEvent::JumpToLive: {
      if (isInReplay()) {
        // Calculer si on est proche du live
        double latencyVal = latency();
        if (latencyVal < 2.0) {
          // Proche du live, transition directe
          transitionTo(LiveReplayState::Live, eventData.event);
          emit liveModeRequested();
        } else {
          // Besoin de catch-up
          transitionTo(LiveReplayState::CatchUpToLive, eventData.event);
          emit seekRequested(m_liveEdgeTime.load(std::memory_order_acquire));
        }
      }
      break;
    }

    case LiveReplayEvent::LiveEdgeAdvanced: {
      // Le live edge avance - pas de transition, juste mise à jour interne
      // Vérifier si on a rattrapé le live en mode CatchUpToLive
      if (currentState == LiveReplayState::CatchUpToLive) {
        double latencyVal = latency();
        if (latencyVal < 1.0) {
          transitionTo(LiveReplayState::Live, eventData.event);
          emit liveModeRequested();
        }
      }
      break;
    }

    case LiveReplayEvent::BufferUnderrun: {
      if (currentState == LiveReplayState::ReplayPlaying ||
          currentState == LiveReplayState::CatchUpToLive) {
        // Passer en pause temporaire ou état d'attente
        LOG_WARNING(Core, "Buffer underrun detected");
        // On reste dans l'état actuel, le pipeline gère le buffering
      }
      break;
    }

    case LiveReplayEvent::BufferRefill: {
      // Le buffer est rechargé, continuer la lecture
      LOG_DEBUG(Core, "Buffer refilled");
      break;
    }

    case LiveReplayEvent::EndOfReplay: {
      // On a atteint le début du cache, rester en pause
      if (currentState == LiveReplayState::ReplayPlaying) {
        transitionTo(LiveReplayState::ReplayPaused, eventData.event);
        emit pauseRequested();
      }
      break;
    }

    case LiveReplayEvent::ErrorOccurred: {
      transitionTo(LiveReplayState::Error, eventData.event);
      emit errorOccurred(eventData.errorMessage);
      break;
    }

    case LiveReplayEvent::CacheUpdated: {
      // La durée du cache a changé - pas de transition nécessaire
      LOG_DEBUG(Core, QString("Cache updated to %1 seconds")
                          .arg(eventData.cacheDuration));
      break;
    }
  }
}

void StateMachineLiveReplay::transitionTo(LiveReplayState newState,
                                          LiveReplayEvent event) {
  LiveReplayState oldState = m_state.exchange(newState, std::memory_order_acq_rel);

  if (oldState != newState) {
    LOG_INFO(Core, QString("State transition: %1 -> %2 (event: %3)")
                       .arg(stateName(oldState))
                       .arg(stateName(newState))
                       .arg(eventName(event)));

    emit stateChanged(static_cast<int>(newState));
    emit transitionOccurred(static_cast<int>(oldState),
                            static_cast<int>(newState),
                            static_cast<int>(event));

    // Vérifier si le statut isInReplay a changé
    bool wasReplay = m_wasInReplay;
    bool nowReplay = isInReplay();
    if (wasReplay != nowReplay) {
      m_wasInReplay = nowReplay;
      emit isInReplayChanged(nowReplay);
    }
  }
}

bool StateMachineLiveReplay::isValidTransition(LiveReplayState from,
                                               LiveReplayState to,
                                               LiveReplayEvent event) const {
  // Table de transitions valides (simplifié)
  // En pratique, la plupart des transitions sont gérées dans processEvent
  Q_UNUSED(from)
  Q_UNUSED(to)
  Q_UNUSED(event)
  return true;
}

void StateMachineLiveReplay::workerLoop() {
  LOG_DEBUG(Core, "Worker thread started");

  while (m_running.load(std::memory_order_acquire)) {
    LiveReplayEventData event;
    bool hasEvent = false;

    {
      QMutexLocker locker(&m_queueMutex);

      // Attendre qu'il y ait des événements (évite le busy-wait)
      while (m_eventQueue.empty() &&
             m_running.load(std::memory_order_acquire)) {
        // Attendre avec timeout pour pouvoir vérifier m_running périodiquement
        m_queueCondition.wait(&m_queueMutex, 100);
      }

      // Vérifier si on doit s'arrêter
      if (!m_running.load(std::memory_order_acquire)) {
        break;
      }

      if (!m_eventQueue.empty()) {
        event = m_eventQueue.front();
        m_eventQueue.pop();
        hasEvent = true;

        if (m_eventQueue.empty()) {
          m_hasEvents.store(false, std::memory_order_release);
        }
      }
    }

    if (hasEvent) {
      // Traiter l'événement sur le thread principal via Qt::QueuedConnection
      QMetaObject::invokeMethod(
          this,
          [this, event]() { processEvent(event); },
          Qt::QueuedConnection);
    }
  }

  LOG_DEBUG(Core, "Worker thread stopped");
}

void StateMachineLiveReplay::enableWorkerThread() {
  if (m_useWorkerThread.load(std::memory_order_acquire)) {
    return;  // Déjà activé
  }

  m_running.store(true, std::memory_order_release);
  m_useWorkerThread.store(true, std::memory_order_release);

  m_workerThread = QThread::create([this]() { workerLoop(); });
  m_workerThread->setObjectName("LiveReplayWorker");
  m_workerThread->start(QThread::NormalPriority);

  LOG_INFO(Core, "Worker thread enabled for StateMachineLiveReplay");
}

void StateMachineLiveReplay::disableWorkerThread() {
  if (!m_useWorkerThread.load(std::memory_order_acquire)) {
    return;  // Déjà désactivé
  }

  m_running.store(false, std::memory_order_release);
  m_useWorkerThread.store(false, std::memory_order_release);

  // Réveiller le thread pour qu'il puisse se terminer
  {
    QMutexLocker locker(&m_queueMutex);
    m_queueCondition.wakeAll();
  }

  if (m_workerThread != nullptr) {
    m_workerThread->quit();
    if (!m_workerThread->wait(2000)) {
      LOG_WARNING(Core, "Worker thread did not stop gracefully, terminating");
      m_workerThread->terminate();
      m_workerThread->wait(1000);
    }
    delete m_workerThread;
    m_workerThread = nullptr;
  }

  LOG_INFO(Core, "Worker thread disabled for StateMachineLiveReplay");
}

}  // namespace blueplayer::core



