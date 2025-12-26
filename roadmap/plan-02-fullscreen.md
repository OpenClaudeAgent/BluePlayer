# Plan 02 - Fullscreen harmonisé

## Contexte
- **Double-clic sur la vidéo** : Fonctionne correctement, affiche la UI en plein écran
- **Bouton fullscreen** : Met dans un mode bizarre sans contrôles, avec un bouton "exit" étrange

Les deux méthodes devraient avoir le même comportement.

## Objectif
Harmoniser le comportement du bouton fullscreen avec celui du double-clic.

## Spécifications

### Comportement attendu
Le bouton fullscreen doit avoir exactement le même comportement que le double-clic :
- Même mode plein écran natif
- UI visible (avec auto-hide)
- Contrôles accessibles
- Sortie via bouton, Escape, ou double-clic

### Code de référence (double-clic)
```qml
onDoubleClicked: {
    var win = Window.window
    if (win) {
        if (win.visibility === Window.FullScreen)
            win.showNormal()
        else
            win.showFullScreen()
    }
}
```

## Fichiers concernés
- `src/ui/components/PlayerControlBar.qml`
- `src/ui/PlayerView.qml`

## Checklist de validation
- [x] Le bouton fullscreen a le même comportement que le double-clic
- [x] La UI est visible en mode plein écran
- [x] Les contrôles sont accessibles en plein écran
- [x] L'auto-hide fonctionne en plein écran (+ curseur caché)
- [x] On peut sortir du plein écran via le bouton
- [x] On peut sortir du plein écran via Escape
- [x] On peut sortir du plein écran via double-clic

## Bonus (ajouté lors de l'implémentation)

- **Auto-hide du curseur** : En mode fullscreen, le curseur disparaît automatiquement avec les contrôles (après 3s d'inactivité). Il réapparaît dès que la souris bouge.
