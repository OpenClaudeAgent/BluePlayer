# Plan 02 - Vitesse de lecture auto-reset

## Contexte
Quand on utilise une vitesse de lecture > 1x pour rattraper le live, et qu'on arrive au bout du buffer live, la vidéo commence à stutter car elle essaie d'aller plus vite que le téléchargement.

## Objectif
Remettre automatiquement la vitesse à 1.0x quand on arrive au live edge.

## Spécifications

### Comportement attendu
- La vitesse > 1x fonctionne normalement en mode VOD/replay
- Quand le lecteur détecte qu'il est arrivé au live edge (mode live actif), la vitesse repasse automatiquement à 1.0x
- L'utilisateur est informé du changement (toast)

### Logique
```
Si mode_live ET vitesse > 1.0:
    vitesse = 1.0
    Afficher toast "Vitesse réinitialisée"
```

## Fichiers concernés
- `src/ui/components/PlayerControlBar.qml`
- `src/ui/PlayerView.qml`
- `src/media/MpvQuickItem.cpp` (détection live edge)

## Checklist de validation
- [ ] La vitesse > 1x fonctionne en mode VOD/replay
- [ ] Quand on arrive au live edge, la vitesse repasse automatiquement à 1.0x
- [ ] Pas de stutter quand on arrive au live
- [ ] L'utilisateur est informé du changement (toast)
- [ ] Le bouton de vitesse affiche bien 1.00x après le reset
