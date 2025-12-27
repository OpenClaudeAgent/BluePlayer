# Plan 09 - Gestion intelligente de la vitesse de lecture

## Dépendances

**⚠️ Ce plan dépend de :** Plan 05 (Gestion Cache/VOD)

La logique de vitesse doit s'appuyer sur le buffer et la durée réelle, pas sur le mode live UI.

---

## Contexte

### Problèmes identifiés

1. **Vitesse > 1x en approchant du live** : Quand on utilise une vitesse > 1x pour rattraper le live et qu'on arrive au bout du buffer, la vidéo stutter car elle essaie d'aller plus vite que le téléchargement.

2. **Vitesse < 1x en mode "live"** : Même problème inversé - si on ralentit en mode live, on accumule du retard mais le système pense qu'on est toujours en live.

3. **Pause en mode live** : Quand on met pause pendant un live, on reste marqué "live" alors que le stream continue sans nous. À la reprise, on n'est plus au live edge.

4. **State machine cassée** : Le mode "live" est un simple booléen UI qui ne reflète pas la réalité du buffer. Toutes les mécaniques de vitesse sont cassées car elles se basent sur ce booléen au lieu du timing réel.

### Problème fondamental

Le **mode live** actuel est un indicateur UI simplifié, pas une mesure précise de la position dans le buffer. Il faut se baser sur :
- La **position actuelle** dans le stream
- La **durée totale** du buffer disponible
- Le **delta temporel** par rapport au live edge (< 2 secondes = au live)

---

## Objectifs

1. Détecter la position réelle par rapport au live edge (basé sur le timing, pas le mode UI)
2. Auto-reset de la vitesse à 1.0x quand on est à < 2s du live edge
3. Gérer correctement la pause (pause = on quitte le live edge)
4. Empêcher les stutters en approchant du live
5. Informer l'utilisateur des changements automatiques

---

## Spécifications

### Définition du "live edge"

```
live_edge = (duration - position) < 2.0 secondes
```

- `duration` : Durée totale du buffer DVR disponible
- `position` : Position actuelle de lecture
- Si le delta est < 2s, on considère qu'on est au live edge

### Comportement attendu

#### Cas 1 : Vitesse > 1x approchant du live
```
Si vitesse > 1.0 ET position approche du live edge (< 5s):
    Réduire progressivement vers 1.0x
    OU
    Reset à 1.0x + toast "Vitesse réinitialisée (live)"
```

#### Cas 2 : Vitesse < 1x en "live"
```
Si vitesse < 1.0 ET on était au live edge:
    Le système doit reconnaître qu'on n'est plus au live
    Passer en mode "VOD/replay" automatiquement
```

#### Cas 3 : Pause pendant un live
```
Si pause ET au live edge:
    Marquer qu'on a quitté le live edge
    À la reprise, on est en mode "replay" (car le stream a continué)
```

#### Cas 4 : Augmentation de vitesse déjà au live
```
Si déjà au live edge ET utilisateur augmente vitesse:
    Permettre jusqu'à ~1.2x (marge de sécurité)
    Au-delà : toast "Déjà au live" + ne pas appliquer
```

### Seuils configurables

| Paramètre | Valeur | Description |
|-----------|--------|-------------|
| LIVE_EDGE_THRESHOLD | 2.0s | Delta max pour être considéré "au live" |
| APPROACHING_LIVE_THRESHOLD | 5.0s | Seuil pour commencer à ralentir |
| MAX_SPEED_AT_LIVE | 1.2x | Vitesse max autorisée au live edge |

---

## Implémentation suggérée

### Nouvelle logique dans MpvQuickItem

```cpp
// Calculer si on est proche du live edge
bool isNearLiveEdge() const {
    if (m_duration <= 0) return false;
    double delta = m_duration - m_position;
    return delta < LIVE_EDGE_THRESHOLD; // 2.0s
}

bool isApproachingLiveEdge() const {
    if (m_duration <= 0) return false;
    double delta = m_duration - m_position;
    return delta < APPROACHING_LIVE_THRESHOLD; // 5.0s
}

// Signal quand on approche du live avec une vitesse > 1x
void checkSpeedAtLiveEdge() {
    if (m_playbackRate > 1.0 && isApproachingLiveEdge()) {
        setPlaybackRate(1.0);
        emit speedAutoReset("Vitesse réinitialisée (live)");
    }
}
```

### Gestion de la pause

```cpp
void MpvQuickItem::pause() {
    if (isNearLiveEdge()) {
        m_wasAtLiveEdgeBeforePause = true;
    }
    // ... pause logic
}

void MpvQuickItem::resume() {
    if (m_wasAtLiveEdgeBeforePause) {
        // On n'est plus au live edge car le stream a continué
        m_wasAtLiveEdgeBeforePause = false;
        emit leftLiveEdge();
    }
    // ... resume logic
}
```

---

## Fichiers concernés

- `src/media/MpvQuickItem.cpp` - Logique de détection et auto-reset
- `src/media/MpvQuickItem.hpp` - Nouveaux signaux et propriétés
- `src/ui/PlayerView.qml` - Gestion des signaux et toasts
- `src/ui/components/PlayerControlBar.qml` - UI de la vitesse

---

## Checklist de validation

### Détection du live edge
- [x] `isNearLiveEdge()` retourne true quand delta < 2s
- [x] `isApproachingLiveEdge()` retourne true quand delta < 5s
- [x] La détection fonctionne indépendamment du mode "live" UI

### Vitesse > 1x
- [x] La vitesse > 1x fonctionne en mode VOD/replay
- [x] Quand on approche du live (< 5s), la vitesse reset à 1.0x
- [x] Toast "Vitesse réinitialisée (live)" s'affiche
- [x] Pas de stutter quand on arrive au live
- [x] Le bouton de vitesse affiche bien 1.00x après le reset

### Vitesse < 1x
- [x] La vitesse < 1x fonctionne normalement
- [x] Si on ralentit depuis le live edge, on passe en mode "replay"
- [x] L'indicateur LIVE/VOD se met à jour correctement

### Pause
- [x] Pause au live edge → on quitte le live edge
- [x] À la reprise, on est en mode "replay"
- [x] L'indicateur LIVE/VOD reflète la réalité

### Protection au live edge
- [x] Au live edge, vitesse max autorisée ~1.2x
- [x] Tentative d'aller plus vite → toast explicatif
- [x] Pas de crash ou comportement bizarre

### Tests généraux
- [x] Pas de régression sur la lecture normale
- [x] Transitions fluides entre les modes
- [x] Toasts non intrusifs
- [x] Performances acceptables
