# Plan 10 - Stratégie Animations Globales

## Contexte

L'application BluePlayer manque actuellement d'une cohérence dans ses animations. Les transitions entre menus, l'ouverture/fermeture de panels, et les interactions utilisateur ne sont pas harmonisées. Une stratégie globale est nécessaire pour créer une expérience fluide et moderne, inspirée des applications Apple.

## Objectifs

### Phase 1 : Audit des animations existantes
- Identifier TOUTES les animations actuelles dans l'application
- Documenter les incohérences (durées, easings, styles)
- Lister les endroits où des animations manquent

### Phase 2 : Définir la stratégie
- Établir une charte d'animation (durées standard, courbes d'easing)
- Définir les patterns d'animation par type d'interaction
- Créer des constantes/variables réutilisables

### Phase 3 : Implémentation
- Refactorer les animations existantes pour suivre la charte
- Ajouter les animations manquantes
- Tester la cohérence globale

## Spécifications

### Points d'animation à auditer

#### Navigation
- [ ] Transition Home → PlayerView
- [ ] Transition Home → PreferencesView
- [ ] Retour vers Home depuis n'importe quelle vue
- [ ] Ouverture/fermeture du panel de préférences

#### Interactions PlayerView
- [ ] Apparition/disparition de la barre de contrôle
- [ ] Hover sur les boutons (volume, fullscreen, etc.)
- [ ] Ouverture du slider de volume
- [ ] Affichage des overlays (titre, loading, etc.)
- [ ] Transition live ↔ replay (state machine)

#### Interactions Home
- [ ] Scroll des catégories horizontales
- [ ] Hover sur les cartes (StreamCard, VideoCard, etc.)
- [ ] Chargement des données (skeleton/placeholder)
- [ ] Apparition des sections

#### Composants génériques
- [ ] Boutons (hover, press, release)
- [ ] Cards (hover, selection)
- [ ] Modals/Dialogs
- [ ] Toasts/Notifications

### Charte d'animation proposée

```javascript
// AppleTheme.js - Section Animations

// Durées standard
const AnimationDurations = {
    instant: 100,      // Feedback immédiat (hover léger)
    fast: 200,         // Transitions rapides (boutons)
    normal: 300,       // Transitions standard (panels)
    slow: 500,         // Transitions importantes (navigation)
    emphasis: 700      // Animations d'emphase
}

// Courbes d'easing (style Apple)
const AnimationEasings = {
    // Entrée douce, sortie rapide
    easeOut: Easing.OutCubic,
    
    // Entrée rapide, sortie douce
    easeIn: Easing.InCubic,
    
    // Symétrique
    easeInOut: Easing.InOutCubic,
    
    // Rebond léger (pour les emphases)
    spring: Easing.OutBack,
    
    // Linéaire (rare, pour les progress bars)
    linear: Easing.Linear
}

// Patterns par type d'interaction
const AnimationPatterns = {
    // Navigation entre vues
    viewTransition: {
        duration: AnimationDurations.slow,
        easing: AnimationEasings.easeInOut
    },
    
    // Hover sur éléments
    hover: {
        duration: AnimationDurations.fast,
        easing: AnimationEasings.easeOut
    },
    
    // Apparition d'overlay
    overlay: {
        duration: AnimationDurations.normal,
        easing: AnimationEasings.easeOut
    },
    
    // Feedback de clic
    press: {
        duration: AnimationDurations.instant,
        easing: AnimationEasings.easeOut
    }
}
```

### Exemples de refactoring

#### Avant (incohérent)
```qml
// Dans un fichier
Behavior on opacity { NumberAnimation { duration: 150 } }

// Dans un autre fichier
Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
```

#### Après (harmonisé)
```qml
// Utilisation des constantes du thème
Behavior on opacity { 
    NumberAnimation { 
        duration: AppleTheme.animation.overlay.duration
        easing.type: AppleTheme.animation.overlay.easing
    } 
}
```

## Fichiers concernés

### À auditer
- `src/ui/main.qml` (navigation principale)
- `src/ui/HomeView.qml`
- `src/ui/PlayerView.qml`
- `src/ui/PreferencesView.qml`
- `src/ui/VideoPlayerApple.qml`
- `src/ui/components/PlayerControlBar.qml`
- `src/ui/components/*.qml` (tous les composants)

### À créer/modifier
- `src/ui/themes/AppleTheme.js` (ajouter section animations)

## Livrables attendus

1. **Document d'audit** : Liste complète des animations avec leur état actuel
2. **Charte d'animation** : Constantes et patterns à suivre
3. **Refactoring** : Mise à jour de tous les fichiers pour utiliser la charte

## Checklist de validation

### Audit
- [ ] Toutes les vues ont été analysées
- [ ] Liste des animations existantes documentée
- [ ] Liste des animations manquantes identifiée
- [ ] Incohérences documentées

### Charte
- [ ] Durées standard définies
- [ ] Courbes d'easing définies
- [ ] Patterns par type d'interaction définis
- [ ] Constantes ajoutées à AppleTheme.js

### Implémentation
- [ ] Animations de navigation harmonisées
- [ ] Animations de hover harmonisées
- [ ] Animations d'overlay harmonisées
- [ ] Animations manquantes ajoutées

### Qualité
- [ ] Pas de saccade ou de lag
- [ ] Animations fluides (60 FPS)
- [ ] Cohérence visuelle sur toute l'app
- [ ] L'expérience "Apple-like" est atteinte

## Notes

Ce plan est conçu pour être exécuté en phases. La Phase 1 (audit) peut être faite rapidement et permettra de mieux estimer l'effort pour les phases suivantes.
