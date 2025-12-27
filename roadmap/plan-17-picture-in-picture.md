# Plan 17 - Picture-in-Picture (PiP)

## Contexte

Le mode Picture-in-Picture permet d'afficher la vidéo dans une petite fenêtre flottante au-dessus des autres applications. C'est une fonctionnalité native de macOS largement utilisée pour le multitâche.

## Objectif

Implémenter le mode PiP pour permettre aux utilisateurs de continuer à regarder un stream tout en utilisant d'autres applications.

## Spécifications

### UI - Bouton PiP

**Emplacement :** Dans la PlayerControlBar, groupe de droite (près du fullscreen).

**Icône :** Icône standard PiP (rectangle avec petit rectangle dans un coin).

```
┌─────────────────────────────────────────────────────────────────┐
│  ▶️  ════════════════════○══════  00:45:23   🔊  [⧉]  [PiP]  ⛶  │
└─────────────────────────────────────────────────────────────────┘
                                                      ↑
                                                 Bouton PiP
```

### Comportement

#### Activation PiP
1. Clic sur le bouton PiP
2. La vidéo se détache dans une fenêtre flottante macOS
3. La fenêtre principale peut afficher un placeholder ou la Home

#### Fenêtre PiP (gérée par macOS)
- Fenêtre flottante au-dessus de toutes les apps
- Redimensionnable (coins)
- Déplaçable (drag)
- Boutons natifs : Play/Pause, Fermer, Retour à l'app
- Position mémorisée par le système

#### Retour à l'app
- Clic sur "Retour à l'app" dans la fenêtre PiP
- Ou clic sur le bouton PiP dans l'app (si visible)
- La vidéo revient dans le PlayerView

### Implémentation technique

**macOS AVKit PiP :**
macOS supporte nativement PiP via `AVPictureInPictureController`. Cependant, cela nécessite d'utiliser `AVPlayerLayer` au lieu de mpv.

**Alternatives pour mpv :**

1. **Fenêtre Qt séparée (recommandée)**
   - Créer une `QQuickWindow` avec le flag `Qt.WindowStaysOnTopHint`
   - Déplacer le `MpvQuickItem` dans cette fenêtre
   - Style minimaliste (pas de barre de titre ou très fine)

2. **NSWindow native**
   - Utiliser Objective-C++ pour créer une NSWindow flottante
   - Plus de contrôle sur l'apparence macOS native

3. **AVPictureInPictureController**
   - Nécessite de capturer le rendu mpv vers un `AVSampleBufferDisplayLayer`
   - Complexe mais rendu PiP 100% natif

### Proposition : Fenêtre Qt flottante

```qml
Window {
    id: pipWindow
    flags: Qt.Window | Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint
    width: 400
    height: 225  // 16:9
    visible: false
    
    MpvQuickItem {
        id: pipPlayer
        anchors.fill: parent
    }
    
    // Mini contrôles
    Row {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        
        Button { text: "⏸" }
        Button { text: "✕"; onClicked: exitPip() }
    }
}
```

## Fichiers concernés

### À modifier
- `src/ui/PlayerView.qml` - Bouton PiP + logique
- `src/ui/components/PlayerControlBar.qml` - Ajouter bouton
- `src/ui/main.qml` - Gestion de la fenêtre PiP

### À créer
- `src/ui/PipWindow.qml` - Fenêtre PiP dédiée

### Potentiellement (si approche native)
- `src/media/PipController.mm` - Wrapper Objective-C++

## Contraintes

- **macOS uniquement** : Le PiP est une feature macOS, prévoir une feature-flag
- **Performance** : Le rendu doit rester fluide dans la petite fenêtre
- **Audio** : L'audio doit suivre la vidéo (pas de duplication)

## Checklist de validation

### UI
- [ ] Bouton PiP visible dans la PlayerControlBar
- [ ] Icône reconnaissable (standard PiP)
- [ ] Bouton grisé si PiP non supporté

### Activation
- [ ] Clic sur PiP ouvre la fenêtre flottante
- [ ] Vidéo s'affiche dans la fenêtre PiP
- [ ] Audio continue normalement
- [ ] Fenêtre principale gère l'absence de vidéo

### Fenêtre PiP
- [ ] Fenêtre toujours au-dessus des autres apps
- [ ] Fenêtre redimensionnable
- [ ] Fenêtre déplaçable
- [ ] Ratio 16:9 maintenu
- [ ] Contrôles minimaux visibles (play/pause, fermer)
- [ ] Taille minimale respectée

### Retour
- [ ] Bouton "retour à l'app" fonctionne
- [ ] Vidéo revient dans le PlayerView
- [ ] État de lecture préservé (position, volume)

### Cas particuliers
- [ ] Fonctionne en live
- [ ] Fonctionne en VOD
- [ ] Gestion correcte du fullscreen (désactiver PiP ou vice versa)
- [ ] Fermeture de l'app ferme aussi le PiP

### Performance
- [ ] Rendu fluide dans la fenêtre PiP
- [ ] Pas de fuite mémoire
- [ ] Transitions sans artefacts
