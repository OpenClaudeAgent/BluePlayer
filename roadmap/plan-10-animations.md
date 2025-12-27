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
- [x] Transition Home → PlayerView
- [x] Transition Home → PreferencesView
- [x] Retour vers Home depuis n'importe quelle vue
- [x] Ouverture/fermeture du panel de préférences

#### Interactions PlayerView
- [x] Apparition/disparition de la barre de contrôle
- [x] Hover sur les boutons (volume, fullscreen, etc.)
- [x] Ouverture du slider de volume
- [x] Affichage des overlays (titre, loading, etc.)
- [x] Transition live ↔ replay (state machine)

#### Interactions Home
- [x] Scroll des catégories horizontales
- [x] Hover sur les cartes (StreamCard, VideoCard, etc.)
- [x] Chargement des données (skeleton/placeholder)
- [x] Apparition des sections

#### Composants génériques
- [x] Boutons (hover, press, release)
- [x] Cards (hover, selection)
- [x] Modals/Dialogs
- [x] Toasts/Notifications

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
- [x] Toutes les vues ont été analysées
- [x] Liste des animations existantes documentée
- [x] Liste des animations manquantes identifiée
- [x] Incohérences documentées

### Charte
- [x] Durées standard définies
- [x] Courbes d'easing définies
- [x] Patterns par type d'interaction définis
- [x] Constantes ajoutées à BlueTheme.js

### Implémentation
- [x] Animations de navigation harmonisées
- [x] Animations de hover harmonisées
- [x] Animations d'overlay harmonisées
- [x] Animations manquantes ajoutées

### Qualité
- [x] Pas de saccade ou de lag
- [x] Animations fluides (60 FPS)
- [x] Cohérence visuelle sur toute l'app
- [x] L'expérience moderne et soignée est atteinte

## Bonus (ajouté lors de l'implémentation)

- **Architecture vues séparées** : Loaders indépendants pour chaque vue (Home, Preferences, Cache, Player) évitant le rechargement des thumbnails lors des navigations
- **Smart refresh des données** : Comparaison intelligente des données avant mise à jour UI pour éviter les blinks inutiles
- **Animation chat synchronisée** : Les contrôles du player suivent l'animation d'ouverture/fermeture du chat
- **Correction zone hover volume** : Le slider de volume ne s'ouvre que sur hover direct du bouton

## Notes

Ce plan est conçu pour être exécuté en phases. La Phase 1 (audit) peut être faite rapidement et permettra de mieux estimer l'effort pour les phases suivantes.
