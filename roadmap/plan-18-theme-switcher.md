# Plan 18 - Thème Clair/Sombre

## Contexte

BluePlayer utilise actuellement un thème sombre défini dans `AppleTheme.js`. Pour une meilleure expérience utilisateur, il serait pertinent de :
1. Supporter un thème clair en plus du thème sombre
2. Suivre automatiquement le thème système macOS
3. Permettre un override manuel dans les préférences

## Objectif

Implémenter un système de thème avec :
- **Mode Auto** : Suit le thème système macOS
- **Mode Manuel** : L'utilisateur choisit clair ou sombre
- **Transition fluide** : Animation lors du changement de thème

## Spécifications

### Logique de sélection

```
1. Vérifier la préférence utilisateur (Config)
   → "auto" : Utiliser le thème système
   → "light" : Forcer le thème clair
   → "dark" : Forcer le thème sombre

2. Si "auto" :
   → Détecter via Qt.styleHints.colorScheme (Qt 6.5+)
   → Ou via NSApp.effectiveAppearance (Objective-C++)
   → Écouter les changements système
```

### UI - Préférences (Plan 12)

```
┌─────────────────────────────────────────────┐
│  Apparence                                  │
│  ─────────────────────────────────────────  │
│                                             │
│  Thème                                      │
│  ┌─────────────────────────────────────┐   │
│  │  ○ Automatique (suit le système)    │   │
│  │  ○ Clair                            │   │
│  │  ○ Sombre                           │   │
│  └─────────────────────────────────────┘   │
│                                             │
│  Aperçu en direct ici...                    │
│                                             │
└─────────────────────────────────────────────┘
```

### Structure AppleTheme.js

**Actuel (thème unique) :**
```javascript
const AppleTheme = {
    background: "#1C1C1E",
    text: "#FFFFFF",
    // ...
}
```

**Nouveau (multi-thème) :**
```javascript
const Themes = {
    dark: {
        background: "#1C1C1E",
        text: "#FFFFFF",
        secondaryText: "#8E8E93",
        accent: "#9147FF",
        cardBackground: "#2C2C2E",
        // ...
    },
    light: {
        background: "#F2F2F7",
        text: "#000000",
        secondaryText: "#6C6C70",
        accent: "#9147FF",
        cardBackground: "#FFFFFF",
        // ...
    }
}

// Thème actif (lié à un context property)
const AppleTheme = Qt.binding(() => {
    return Settings.theme === "light" ? Themes.light : Themes.dark
})
```

### Couleurs à définir

| Propriété | Sombre | Clair |
|-----------|--------|-------|
| background | #1C1C1E | #F2F2F7 |
| secondaryBackground | #2C2C2E | #FFFFFF |
| tertiaryBackground | #3A3A3C | #E5E5EA |
| text | #FFFFFF | #000000 |
| secondaryText | #8E8E93 | #6C6C70 |
| accent | #9147FF | #9147FF |
| success | #30D158 | #34C759 |
| error | #FF453A | #FF3B30 |
| cardBackground | #2C2C2E | #FFFFFF |
| cardBorder | #3A3A3C | #E5E5EA |
| controlBackground | #3A3A3C | #E5E5EA |

### Détection du thème système

**Qt 6.5+ :**
```qml
Connections {
    target: Qt.styleHints
    function onColorSchemeChanged() {
        if (Settings.theme === "auto") {
            applyTheme(Qt.styleHints.colorScheme === Qt.Dark ? "dark" : "light")
        }
    }
}
```

**Objective-C++ (fallback) :**
```objc
// Dans un helper
+ (BOOL)isDarkMode {
    if (@available(macOS 10.14, *)) {
        NSAppearance *appearance = NSApp.effectiveAppearance;
        NSAppearanceName name = [appearance bestMatchFromAppearancesWithNames:@[
            NSAppearanceNameAqua, NSAppearanceNameDarkAqua
        ]];
        return [name isEqualToString:NSAppearanceNameDarkAqua];
    }
    return NO;
}
```

## Dépendances

| Ce plan | Dépend de | Raison |
|---------|-----------|--------|
| 18 | Plan 12 (Préférences) | Le sélecteur de thème sera dans les préférences |

## Fichiers concernés

### À modifier
- `src/ui/themes/AppleTheme.js` - Refactorer pour multi-thème
- `src/ui/PreferencesView.qml` - Ajouter sélecteur de thème
- `src/core/Config.cpp/hpp` - Stocker la préférence
- `src/main.mm` - Détection thème système au démarrage
- Tous les fichiers QML utilisant AppleTheme (migration progressive)

### À créer (optionnel)
- `src/core/ThemeManager.cpp/hpp` - Gestion centralisée du thème

## Checklist de validation

### Configuration
- [ ] Préférence "theme" stockée dans Config (auto/light/dark)
- [ ] Valeur par défaut = "auto"
- [ ] Préférence persistée après redémarrage

### Détection système
- [ ] Thème système détecté au démarrage
- [ ] Changement de thème système détecté en temps réel
- [ ] Fonctionne sur macOS 10.14+

### UI Préférences
- [ ] Sélecteur de thème visible (3 options)
- [ ] Option actuelle mise en évidence
- [ ] Changement appliqué immédiatement (ou après confirmation)

### Thèmes
- [ ] Thème sombre complet et cohérent
- [ ] Thème clair complet et cohérent
- [ ] Toutes les vues supportent les deux thèmes
- [ ] Pas de texte illisible (contraste suffisant)
- [ ] Couleur accent identique dans les deux thèmes

### Transition
- [ ] Animation fluide lors du changement (optionnel)
- [ ] Pas de flash blanc/noir brutal
- [ ] État de l'app préservé lors du changement

### Tests
- [ ] Testé en mode sombre
- [ ] Testé en mode clair
- [ ] Testé en mode auto avec changement système
- [ ] L'app compile sans erreur
- [ ] Pas de régression fonctionnelle
