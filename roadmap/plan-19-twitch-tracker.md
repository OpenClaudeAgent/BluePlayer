# Plan 19 - Intégration Twitch Tracker

## Contexte

Twitch Tracker (twitchtracker.com) est un site externe qui fournit des statistiques détaillées sur les streamers : historique de viewers, heures de stream, croissance, etc. Intégrer un accès à ces statistiques enrichirait l'expérience BluePlayer.

## Objectif

Permettre aux utilisateurs d'accéder aux statistiques Twitch Tracker d'un streamer depuis le PlayerView, avec une logique différente selon que le streamer est live ou non.

## Spécifications

### Comportement selon l'état

#### Cas 1 : Streamer en LIVE
```
[Recherche] → [Clic sur stream live] → [PlayerView s'ouvre]
                                              ↓
                                    [Bouton "Stats" visible]
                                              ↓
                                    [Clic] → [Ouvre TwitchTracker]
```

L'utilisateur regarde le live et peut accéder aux stats via un bouton.

#### Cas 2 : Streamer OFFLINE (évolution future)
```
[Recherche] → [Résultat offline] → [Clic] → [Ouvre TwitchTracker directement]
```

Pour les streamers offline, le clic pourrait ouvrir directement leurs stats (pas de vidéo à regarder).

### UI - Bouton dans PlayerView

**Emplacement :** Zone d'information du stream (près du nom) ou dans un menu "Plus".

**Design :**
```
┌─────────────────────────────────────────────────────────────┐
│  [Avatar] StreamerName                                      │
│  Playing: Game Title                          [♥] [📊]      │
│  5.2K viewers                                     ↑         │
│                                             Stats button    │
└─────────────────────────────────────────────────────────────┘
```

### Action du bouton

**Ouvrir dans le navigateur par défaut :**
```cpp
QDesktopServices::openUrl(QUrl("https://twitchtracker.com/" + channelName));
```

**Alternative : WebView intégrée (plus complexe)**
- Ouvrir une fenêtre/panel avec une WebView
- Afficher twitchtracker.com/channelName
- Avantage : Reste dans l'app
- Inconvénient : Complexité, maintenance

### Recommandation

**Phase 1 : Ouvrir dans le navigateur** (simple, rapide à implémenter)
**Phase 2 (optionnelle) : WebView intégrée** (si demandé)

### URL Twitch Tracker

Format : `https://twitchtracker.com/{channel_login}`

Exemple : `https://twitchtracker.com/xqc`

### Gestion des channels offline (évolution)

Pour permettre de naviguer vers des channels offline via Twitch Tracker :

1. **Recherche étendue** : Afficher aussi les channels offline dans les résultats
2. **Indication visuelle** : Badge "Offline" sur les résultats
3. **Action différente** : Clic → Ouvre Twitch Tracker au lieu du player

```
┌─────────────────────────────────────────────┐
│  🔴 xQc                    LIVE  12.5K      │  → Ouvre Player
│  ⚫ Streamer2              OFFLINE          │  → Ouvre TwitchTracker
│  🔴 Streamer3              LIVE  3.2K       │  → Ouvre Player
└─────────────────────────────────────────────┘
```

## Fichiers concernés

### À modifier
- `src/ui/PlayerView.qml` - Ajouter bouton stats

### Optionnel (channels offline)
- `src/ui/components/SearchResults.qml` - Afficher channels offline
- `src/api/twitch/TwitchService.cpp` - Recherche incluant offline

## Sous-tâches

| # | Sous-tâche | Description |
|---|------------|-------------|
| 19.1 | Bouton Stats (PlayerView) | Ajouter le bouton qui ouvre TwitchTracker |
| 19.2 | Channels offline (Recherche) | Afficher les channels offline dans la recherche |
| 19.3 | Navigation différenciée | Live → Player, Offline → TwitchTracker |

## Priorité des sous-tâches

| Priorité | Sous-tâche | Dépendances |
|----------|------------|-------------|
| 1 | 19.1 - Bouton Stats | Aucune |
| 2 | 19.2 - Channels offline | Plan 6 (Search) ✅ |
| 3 | 19.3 - Navigation différenciée | 19.2 |

## Checklist de validation

### Bouton Stats (19.1)
- [ ] Bouton visible dans PlayerView (icône graphique/stats)
- [ ] Clic ouvre le navigateur par défaut
- [ ] URL correcte : twitchtracker.com/{channel}
- [ ] Fonctionne pour tous les streamers

### Channels offline (19.2) - Optionnel
- [ ] Recherche retourne aussi les channels offline
- [ ] Badge "Offline" visible sur les résultats offline
- [ ] Différenciation claire live vs offline

### Navigation différenciée (19.3) - Optionnel
- [ ] Clic sur live → Ouvre PlayerView
- [ ] Clic sur offline → Ouvre TwitchTracker
- [ ] Comportement cohérent et intuitif

### Tests
- [ ] L'app compile sans erreur
- [ ] Le bouton fonctionne avec différents streamers
- [ ] Pas de régression sur la recherche existante
