# Status - Tâche 9 : Vitesse lecture intelligente

**Branche** : `feature/playback-speed`
**Statut** : ⏸️ En pause (WIP)
**Date** : 2025-12-27

## Ce qui a été implémenté

### MpvQuickItem.hpp
- Constantes de seuil : `LIVE_EDGE_THRESHOLD` (2s), `APPROACHING_LIVE_THRESHOLD` (5s), `MAX_SPEED_AT_LIVE` (1.2x)
- Méthodes `isNearLiveEdge()` et `isApproachingLiveEdge()`
- Signaux `speedAutoReset(QString)` et `leftLiveEdge()`
- Membre `m_wasAtLiveEdgeBeforePause`

### MpvQuickItem.cpp
- Implémentation des méthodes de détection live edge
- `pause()` : track si on était au live edge
- `resume()` : émet `leftLiveEdge` si on était au live edge avant pause
- `setPlaybackRate()` : limite à 1.2x au live edge, quitte le mode live si < 1.0x
- `processPropertyChange()` : auto-reset vitesse à 1.0x quand on approche du live avec vitesse > 1.0

### PlayerView.qml
- Handler `onSpeedAutoReset` : affiche toast + sync playbackRate
- Handler `onLeftLiveEdge` : affiche toast "Mode replay"
- Handler `onPlaybackRateChanged` : sync UI avec mpvPlayer

## À tester

| # | Critère |
|---|---------|
| 1 | Vitesse > 1x en mode replay fonctionne |
| 2 | Auto-reset à 1.0x quand on approche du live (< 5s) |
| 3 | Vitesse limitée à 1.2x au live edge |
| 4 | Pause au live → mode replay à la reprise |
| 5 | Ralentir au live → passe en mode replay |
| 6 | Pas de stutter en rattrapant le live |

## Prochaines étapes

1. Tester manuellement chaque critère
2. Valider avec l'utilisateur
3. Finaliser (plan, roadmap, changelog, tag v0.9.0)
