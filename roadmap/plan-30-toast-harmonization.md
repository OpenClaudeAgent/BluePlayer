# Plan 30 - Harmonisation des Toasts

## Contexte

Le Plan 16 a introduit un nouveau design de toast moderne (forme pill, taille dynamique, fond semi-transparent). Cependant, ce toast n'est pas affiché pour tous les contrôles du player. L'utilisateur n'a pas de feedback visuel pour certaines actions comme le mute/unmute.

## Objectif

Harmoniser l'affichage des toasts pour que chaque action utilisateur sur les contrôles du player génère un feedback visuel via le toast.

## Spécifications

### Contrôles à couvrir

| Contrôle | Action | Toast actuel | Toast attendu |
|----------|--------|--------------|---------------|
| Volume | Mute | ❌ Aucun | "Volume: Muted" / "Volume: XX%" |
| Volume | Unmute | ❌ Aucun | "Volume: XX%" |
| Volume | Slider change | ❌ Aucun | "Volume: XX%" |
| Vitesse | +/- | ✅ "Speed 1.25x" | OK |
| Vitesse | Reset (R) | ✅ "Speed 1.00x" | OK |
| Qualité | Change | ✅ "Quality: 1080p" | OK |
| HW/SW | Toggle | ✅ "Hardware/Software decoding" | OK |
| Crop/Fit | Toggle | ✅ "Crop/Fit mode" | OK |
| Chat | Toggle | ❌ Aucun | "Chat: On" / "Chat: Off" |
| Fullscreen | Toggle | ❌ Aucun | Optionnel (visuel évident) |

### Comportement

1. Le toast apparaît pendant ~1.5s puis disparaît
2. Un nouveau toast remplace le précédent immédiatement
3. Le toast utilise le design introduit en Plan 16 (pill, dynamique, 75% opacity)

### Icônes suggérées

| Type | Icône |
|------|-------|
| Volume | 🔊 / 🔇 |
| Vitesse | ⏩ |
| Qualité | 📺 |
| Chat | 💬 |

## Fichiers concernés

### À modifier
- `src/ui/PlayerView.qml` - Ajouter les appels `toast.show()` manquants
- `src/ui/components/VolumeControl.qml` - Émettre signal pour toast (si pas déjà fait)

### Référence (ne pas modifier)
- `src/ui/themes/BlueTheme.js` - Design tokens déjà définis

## Checklist de validation

### Volume
- [ ] Toast affiché au mute
- [ ] Toast affiché au unmute
- [ ] Toast affiché au changement de volume (slider)

### Chat
- [ ] Toast affiché à l'activation du chat
- [ ] Toast affiché à la désactivation du chat

### Consistance
- [ ] Tous les toasts utilisent le même design (pill, 75% opacity)
- [ ] Les messages sont traduits (qsTr)
- [ ] Pas de régression sur les toasts existants

### Tests
- [ ] L'app compile sans erreur
- [ ] Pas de spam de toasts (debounce si nécessaire)
