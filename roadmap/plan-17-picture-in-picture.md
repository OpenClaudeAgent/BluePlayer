# Plan 17 - Picture-in-Picture (PiP)

## Contexte

Le mode Picture-in-Picture permet d'afficher la vidéo dans une petite fenêtre flottante au-dessus des autres applications. C'est une fonctionnalité native de macOS largement utilisée pour le multitâche.

## Objectif

Implémenter le mode PiP pour permettre aux utilisateurs de continuer à regarder un stream tout en utilisant d'autres applications.

## Approche technique retenue

**Fenêtre Qt flottante** (option 1 du plan initial) :
- Créer une `Window` QML avec le flag `Qt.WindowStaysOnTopHint`
- Réutiliser le même `MpvQuickItem` (pas de duplication de rendu)
- Style minimaliste avec contrôles au hover
- Plus simple et stable qu'une intégration AVKit native

> Note : L'intégration AVPictureInPictureController nécessiterait de capturer le rendu mpv vers AVSampleBufferDisplayLayer, ce qui est complexe et risqué pour la stabilité.

## Spécifications

### 1. Bouton PiP dans PlayerControlBar

**Emplacement :** Entre le bouton Volume et le bouton Fullscreen (groupe de droite).

**Composant :** Réutilise `ControlButton` existant.

**Icône :** Deux rectangles imbriqués (standard PiP).

```
Layout actuel de droite :
[Quality] [Volume] [Fullscreen]

Nouveau layout :
[Quality] [Volume] [PiP] [Fullscreen]
```

**États du bouton :**
- Normal : Icône blanche sur fond semi-transparent
- Hover : Légère mise à l'échelle (1.05x)
- Active (PiP ouvert) : Fond accent (BlueTheme.accent)
- Désactivé : Opacité réduite (quand en fullscreen)

**Raccourci clavier :** `P` (pour Picture-in-Picture)

### 2. PipWindow - Fenêtre flottante

**Dimensions :**
- Taille par défaut : 400x225 (ratio 16:9)
- Taille minimale : 320x180
- Taille maximale : 640x360

**Comportement :**
- Toujours au-dessus des autres fenêtres (`WindowStaysOnTopHint`)
- Sans barre de titre (`FramelessWindowHint`)
- Redimensionnable (coins) avec **ratio 16:9 forcé**
- Déplaçable (drag n'importe où)
- Coins arrondis (12px)
- Ombre portée subtile

**Position initiale :**
- Coin inférieur droit de l'écran
- Marge de 24px depuis les bords

**Mémorisation position/taille :**
- Sauvegarder dans QSettings quand fermée
- Restaurer à la réouverture

### 3. Mini-contrôles PipWindow

**Comportement :** Apparaissent au hover, disparaissent après 2s.

**Layout :**
```
┌─────────────────────────────────────┐
│                                [✕]  │ ← Bouton fermer (top-right)
│                                     │
│              [VIDEO]                │
│                                     │
│         [⏸]  [↗]                    │ ← Contrôles (bottom-center)
│          ↑    ↑                     │
│       Pause  Retour app             │
└─────────────────────────────────────┘
```

**Boutons :**
1. **Fermer (✕)** - Ferme le PiP, **arrête la lecture**, retour Home
2. **Play/Pause (▶/⏸)** - Toggle lecture
3. **Retour à l'app (↗)** - Ferme PiP, remet la vidéo dans PlayerView (lecture continue)

**Style des contrôles :**
- Fond : #80000000 (noir 50% opacité)
- Coins arrondis : 8px
- Taille boutons : 32x32
- Animation fade : BlueTheme.animHoverDuration (150ms)

### 4. Gestion des états

**PiP et Fullscreen sont mutuellement exclusifs :**
- Si en fullscreen → bouton PiP désactivé
- Si PiP activé en fullscreen → quitter fullscreen d'abord

**Transitions :**

| Action | Depuis | Vers | Comportement |
|--------|--------|------|--------------|
| Clic PiP | Normal | PiP | Fenêtre principale montre placeholder, vidéo dans PiP |
| Clic PiP | PiP | Normal | Ferme PiP, vidéo revient dans PlayerView |
| Clic Retour | PiP | Normal | Idem |
| Clic Fermer | PiP | - | Ferme PiP, stop lecture, retour Home |
| Double-clic PiP | PiP | Normal | Shortcut : ferme PiP, vidéo revient |
| Touche P | Normal | PiP | Toggle PiP |
| Touche P | PiP | Normal | Toggle PiP |
| Touche Esc | PiP | Normal | Ferme PiP, vidéo revient |

**Fenêtre principale en mode PiP :**
- Option A : Affiche un placeholder "Vidéo en PiP" avec bouton pour revenir
- Option B : Retourne automatiquement à Home
- **Choix : Option A** (meilleure UX, garde le contexte)

### 5. Placeholder "Vidéo en PiP"

**Affiché dans PlayerView quand PiP actif :**

```
┌─────────────────────────────────────────┐
│                                         │
│              🎬                         │
│     "Vidéo en Picture-in-Picture"       │
│                                         │
│     [Revenir ici]                       │
│                                         │
└─────────────────────────────────────────┘
```

**Style :**
- Fond : BlueTheme.windowBackground
- Icône : Emoji ou Canvas (style PiP)
- Texte : BlueTheme.primaryText, 18px
- Bouton : Style secondaire, action → ferme PiP

### 6. Animations

Toutes les animations utilisent les tokens BlueTheme :

| Animation | Durée | Easing |
|-----------|-------|--------|
| Apparition contrôles PiP | animHoverDuration (150ms) | OutCubic |
| Disparition contrôles PiP | animHoverDuration (150ms) | OutCubic |
| Ouverture fenêtre PiP | animPanelDuration (300ms) | OutCubic |
| Fermeture fenêtre PiP | animPanelDuration (300ms) | InCubic |
| Hover bouton | animHoverDuration (150ms) | OutCubic |

### 7. Préservation d'état

Lors du passage en PiP et retour :
- ✅ Position de lecture (automatique, même player)
- ✅ Volume
- ✅ État mute
- ✅ État pause
- ✅ Mode live vs replay
- ✅ Qualité sélectionnée

## Fichiers concernés

### À modifier
| Fichier | Modification |
|---------|--------------|
| `src/ui/components/PlayerControlBar.qml` | Ajouter bouton PiP |
| `src/ui/PlayerView.qml` | Logique PiP + placeholder |
| `src/ui/main.qml` | Gestion fenêtre PiP globale |
| `src/ui/themes/BlueTheme.js` | (si nouvelles constantes nécessaires) |

### À créer
| Fichier | Description |
|---------|-------------|
| `src/ui/PipWindow.qml` | Fenêtre PiP flottante |
| `src/ui/components/PipPlaceholder.qml` | Placeholder "Vidéo en PiP" |

## Contraintes techniques

1. **macOS uniquement** : Le comportement `WindowStaysOnTopHint` fonctionne sur macOS. Tester sur les autres plateformes si nécessaire.

2. **Chat séparé** : Le chat Twitch reste dans la fenêtre principale, pas dans la fenêtre PiP. L'utilisateur peut garder le chat ouvert tout en utilisant d'autres apps avec le PiP.

3. **Un seul MpvQuickItem** : Ne pas dupliquer le player. Utiliser `parent` reparenting pour déplacer le MpvQuickItem entre les fenêtres.

4. **Performance** : Le rendu doit rester fluide. Pas de redimensionnement du rendu mpv pendant le reparenting.

5. **Focus clavier** : La fenêtre PiP ne doit pas voler le focus. Les raccourcis claviers restent actifs dans la fenêtre principale.

## Checklist de validation

### UI - Bouton PiP
- [x] Bouton PiP visible dans la PlayerControlBar (entre Volume et Fullscreen)
- [x] Icône reconnaissable (deux rectangles imbriqués)
- [x] Tooltip "Picture-in-Picture (P)"
- [x] État hover avec animation
- [x] État actif quand PiP ouvert
- [x] Bouton grisé si en fullscreen

### Activation PiP
- [x] Clic sur PiP ouvre la fenêtre flottante
- [x] Vidéo s'affiche dans la fenêtre PiP (même player, reparenté)
- [x] Audio continue normalement
- [x] Fenêtre principale affiche placeholder

### Fenêtre PiP
- [x] Fenêtre toujours au-dessus des autres apps
- [x] Fenêtre sans barre de titre (frameless)
- [x] Fenêtre redimensionnable avec ratio 16:9 forcé
- [x] Fenêtre déplaçable (drag)
- [x] Coins arrondis (16px - ajusté pour meilleur rendu)
- [x] Taille minimale respectée (320x180)
- [x] Position/taille mémorisées entre sessions

### Contrôles PiP
- [x] Contrôles apparaissent au hover
- [x] Contrôles disparaissent après 2s d'inactivité
- [x] Bouton Play/Pause fonctionne
- [x] Bouton Fermer ferme PiP et arrête lecture
- [x] Bouton Retour ferme PiP et remet vidéo dans app

### Retour à l'app
- [x] Clic sur "Retour à l'app" fonctionne
- [x] Double-clic sur fenêtre PiP ferme et revient
- [x] Touche P toggle le mode PiP
- [x] Touche Escape ferme PiP et revient
- [x] Vidéo revient dans le PlayerView
- [x] État de lecture préservé (position, volume, qualité)

### Cas particuliers
- [x] Fonctionne en live
- [x] Fonctionne en VOD/replay
- [x] Gestion correcte du fullscreen (mutuellement exclusif)
- [x] Fermeture de l'app ferme aussi le PiP
- [x] PiP fonctionne avec le chat ouvert (chat reste dans app principale)

### Performance
- [x] Rendu fluide dans la fenêtre PiP
- [x] Pas de fuite mémoire (vérifier avec Instruments)
- [x] Transitions sans artefacts visuels
- [x] Reparenting du MpvQuickItem sans freeze
