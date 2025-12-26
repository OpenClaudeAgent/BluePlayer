# Plan 11 - Sélection de la Qualité du Stream

## Contexte

Actuellement, BluePlayer sélectionne automatiquement une qualité par défaut pour les streams (probablement 1080p ou la meilleure disponible). L'utilisateur n'a aucun contrôle sur la qualité du flux vidéo, ce qui peut être problématique :
- Connexion limitée → besoin de baisser la qualité
- Préférence personnelle → certains préfèrent 720p pour économiser la bande passante
- Mode data saver → sur certains réseaux

## Objectifs

1. **Ajouter un menu de sélection de qualité** dans le PlayerView
2. **Afficher les qualités disponibles** pour le stream/VOD actuel
3. **Permettre de changer la qualité à la volée** sans interrompre le flux
4. **Préparer l'intégration** avec les préférences (qualité par défaut)

## Spécifications

### UI - Bouton qualité dans la barre de contrôle

```
┌─────────────────────────────────────────────────────────────────┐
│  ▶️  ════════════════════○══════  00:45:23   🔊  [1080p]  ⛶   │
└─────────────────────────────────────────────────────────────────┘
                                                    ↑
                                              Bouton qualité
```

### UI - Menu popup de sélection

```
        ┌──────────────────┐
        │  Qualité         │
        │  ──────────────  │
        │  ○ Auto          │
        │  ● 1080p60       │  ← actuellement sélectionné
        │  ○ 1080p         │
        │  ○ 720p60        │
        │  ○ 720p          │
        │  ○ 480p          │
        │  ○ 360p          │
        │  ○ 160p          │
        └──────────────────┘
```

### Comportement attendu

1. **Clic sur le bouton qualité** → Ouvre le menu popup
2. **Sélection d'une qualité** → Change le flux, ferme le menu
3. **Clic en dehors** → Ferme le menu sans changer
4. **Échap** → Ferme le menu sans changer

### Option "Auto"
- Laisse mpv/HLS choisir la meilleure qualité adaptative
- Affiche "Auto" sur le bouton
- Comportement par défaut actuel

### Intégration avec mpv

mpv supporte le changement de qualité HLS via la propriété `vid` ou en manipulant les playlists. Pour Twitch :

```cpp
// Option 1 : Changer la playlist HLS
// L'URL Twitch HLS contient la qualité dans le chemin
// On peut reconstruire l'URL avec une qualité différente

// Option 2 : Utiliser les variants HLS
// mpv expose les pistes vidéo disponibles
// On peut switcher entre elles
```

### API Twitch - Récupération des qualités

L'API Twitch/UsherTV retourne les qualités disponibles :

```json
{
  "streams": [
    {"quality": "1080p60", "url": "..."},
    {"quality": "1080p", "url": "..."},
    {"quality": "720p60", "url": "..."},
    {"quality": "720p", "url": "..."},
    {"quality": "480p", "url": "..."},
    {"quality": "360p", "url": "..."},
    {"quality": "160p", "url": "..."},
    {"quality": "audio_only", "url": "..."}
  ]
}
```

## Analyse technique requise

### Questions à résoudre
1. Comment mpv gère-t-il actuellement la qualité ?
2. Peut-on changer de qualité sans recharger le stream ?
3. Comment récupérer la liste des qualités disponibles depuis TwitchService ?
4. Faut-il modifier MpvQuickItem pour exposer cette fonctionnalité ?

### Fichiers à analyser
- `src/media/MpvQuickItem.cpp` - Gestion mpv
- `src/api/twitch/TwitchService.cpp` - Récupération stream URL
- `src/api/twitch/TwitchApiClient.cpp` - Appels API

## Fichiers concernés

### À modifier
- `src/ui/components/PlayerControlBar.qml` - Ajouter bouton qualité
- `src/ui/PlayerView.qml` - Gérer le popup
- `src/media/MpvQuickItem.cpp/hpp` - Exposer les qualités
- `src/api/twitch/TwitchService.cpp/hpp` - Retourner les qualités disponibles

### À créer
- `src/ui/components/QualitySelector.qml` - Composant popup de sélection

## Dépendances

### Ce plan est requis par
- **Plan 12 (Préférences)** : Pour implémenter "qualité par défaut"

### Ce plan dépend de
- Aucune dépendance bloquante

## Checklist de validation

### UI
- [ ] Bouton qualité visible dans la barre de contrôle
- [ ] Bouton affiche la qualité actuelle (ex: "1080p")
- [ ] Menu popup s'ouvre au clic
- [ ] Menu liste toutes les qualités disponibles
- [ ] Qualité actuelle est mise en évidence
- [ ] Menu se ferme après sélection
- [ ] Menu se ferme au clic en dehors
- [ ] Menu se ferme avec Échap

### Fonctionnel
- [ ] Changement de qualité fonctionne
- [ ] Pas d'interruption visible lors du changement
- [ ] Option "Auto" disponible
- [ ] La qualité persiste pendant la session

### Cas particuliers
- [ ] Fonctionne pour les streams live
- [ ] Fonctionne pour les VOD
- [ ] Gère le cas où peu de qualités sont disponibles
- [ ] Affichage correct si stream audio-only

### Technique
- [ ] Pas de fuite mémoire
- [ ] Transitions fluides
- [ ] Logs appropriés pour debug

## Notes pour plan-12

Une fois ce plan implémenté, le plan-12 (Préférences) pourra ajouter :
- Un paramètre "Qualité par défaut" (Auto, 1080p, 720p, etc.)
- Ce paramètre sera lu au démarrage d'un stream
- L'utilisateur pourra toujours override manuellement

## Estimation

- **Complexité** : Moyenne à élevée
- **Points d'incertitude** : Intégration mpv, changement sans interruption
- **Suggestion** : Commencer par l'analyse technique (spike)
