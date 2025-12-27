# Plan 25 - Audit UI/UX & Design System

## Mon Rôle

Je suis un **UI Designer** spécialisé dans la création d'interfaces visuellement élégantes et modernes. Mon analyse porte exclusivement sur les aspects visuels, l'expérience utilisateur et la cohérence du design.

> *"Le bon design est invisible. L'utilisateur ne remarque pas l'interface, il accomplit simplement ses objectifs avec plaisir."*

---

## Analyse du Design System Actuel

### BlueTheme.js - Évaluation

| Aspect | Score | Commentaire |
|--------|-------|-------------|
| Palette de couleurs | 7/10 | Cohérente mais incomplète |
| Système d'espacement | 8/10 | Bien structuré (8/16/24px) |
| Typographie | 6/10 | Font unique, manque de hiérarchie définie |
| Coins arrondis | 4/10 | `cornerRadius: 22` trop agressif |
| Élévation/Ombres | 3/10 | Quasi inexistant |

---

## 1. Palette de Couleurs

### Analyse Actuelle

```javascript
// Palette existante - Teintes bleutées cohérentes ✓
windowBackground: "#03050b"   // Très sombre, presque noir
gradientStart: "#040b15"      // Gradient subtil
gradientEnd: "#0b1727"        
surface: "#141c2a"            // Surface des cartes
surfaceSoft: "#1d2533"        // Surface hover
accent: "#5bc0ff"             // Bleu vif - Bonne saturation
primaryText: "#f6f7fa"        // Blanc cassé - Bon contraste
secondaryText: "#aeb9c9"      // Gris bleuté
mutedText: "#7d89a4"          // Attention: contraste limite
```

### Problèmes Identifiés

#### 1.1 Contraste Insuffisant sur `mutedText`
```
mutedText (#7d89a4) sur windowBackground (#03050b)
→ Ratio de contraste: ~4.2:1
→ WCAG AA exige 4.5:1 pour texte normal
```

**Recommandation:** Éclaircir à `#8a96b0` (ratio ~5:1)

#### 1.2 Palette Incomplète - Couleurs Manquantes

Le thème ne couvre pas tous les cas d'usage de l'application :

| Cas d'usage | Couleur actuelle | Problème |
|-------------|------------------|----------|
| Fond player | `#000000` hardcodé | Pas dans le thème |
| Contrôles player | `#1AFFFFFF`, `#33FFFFFF` | Hardcodé partout |
| Chat background | `#1C1C1E` | Style iOS, incohérent |
| Chat surface | `#2C2C2E` | Style iOS, incohérent |
| Hover cards | `#1a2230`, `#252d3d` | Hardcodé dans chaque carte |
| Barre recherche | `#0d1117`, `#161d28` | Hardcodé |

#### 1.3 Deux Design Languages en Conflit

L'application mélange deux langages visuels :

| Zone | Style | Couleurs |
|------|-------|----------|
| Home, Cards | BlueTheme (bleuté) | `#141c2a`, `#1d2533` |
| Chat, Player | iOS/Apple (gris neutre) | `#1C1C1E`, `#2C2C2E` |

**Recommandation:** Unifier vers BlueTheme partout.

---

## 2. Typographie

### Analyse Actuelle

```javascript
fontFamily: "SF Pro Display" → "Helvetica Neue" → "Inter" → fallback
// Aucune échelle de tailles définie dans le thème
```

### Problèmes

| Problème | Exemple | Impact |
|----------|---------|--------|
| Pas d'échelle définie | Tailles dispersées: 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 56px | Incohérence |
| Pas de poids définis | font.bold vs font.weight: 400/500/600 mélangés | Confusion |
| Line-height non défini | Parfois 1.6, parfois implicite | Lisibilité variable |

### Échelle Typographique Recommandée

```javascript
// À ajouter dans BlueTheme.js
var fontSizeHero = 48        // Titre principal (login)
var fontSizeH1 = 24          // Titres de section
var fontSizeH2 = 20          // Sous-sections
var fontSizeH3 = 16          // Titres de cartes
var fontSizeBody = 14        // Texte principal
var fontSizeCaption = 12     // Texte secondaire
var fontSizeSmall = 11       // Labels, badges
var fontSizeMicro = 10       // Timestamps, métadonnées

var fontWeightRegular = 400
var fontWeightMedium = 500
var fontWeightSemibold = 600
var fontWeightBold = 700

var lineHeightNormal = 1.4
var lineHeightRelaxed = 1.6
```

---

## 3. Système d'Espacement

### Analyse Actuelle ✓ Bon

```javascript
spacingSmall: 8    // ✓ Base 8px
spacingMedium: 16  // ✓ x2
spacingLarge: 24   // ✓ x3
spacing: 14        // ⚠️ Rompt le système (ni 12 ni 16)
```

### Problème: Valeurs Hors Système

De nombreuses valeurs arbitraires dans le code :

```qml
// Exemples de violations
anchors.margins: 12          // Devrait être 8 ou 16
spacing: 6                   // Devrait être 8
topPadding: 20              // Devrait être 16 ou 24
Layout.topMargin: 50        // Arbitraire
```

### Système Recommandé (Base 8)

```javascript
var spacing2 = 2    // Micro-ajustements
var spacing4 = 4    // Entre éléments liés
var spacing8 = 8    // Standard petit (spacingSmall)
var spacing12 = 12  // Intermédiaire
var spacing16 = 16  // Standard moyen (spacingMedium)
var spacing24 = 24  // Standard grand (spacingLarge)
var spacing32 = 32  // Entre sections
var spacing48 = 48  // Marges de page
var spacing64 = 64  // Grands espaces
```

---

## 4. Coins Arrondis

### Problème Majeur

```javascript
cornerRadius: 22     // Beaucoup trop grand
heroCornerRadius: 30 // Encore plus grand
```

**Conséquences:**
- Les cartes de 180x220px avec radius 22 semblent "gonflées"
- Perte d'espace utilisable aux coins
- Style "bulle" peu professionnel

### Incohérence des Radius

| Composant | Radius Actuel | Cohérent? |
|-----------|---------------|-----------|
| Cards | 12px | ✓ Bon |
| BlueCard | 8px | ✓ Bon |
| Buttons | 14px | ⚠️ Différent |
| Login card | 8px | ✓ Bon |
| Search bar | 12px | ✓ Bon |
| Badges | 9-10px (pills) | ✓ OK pour pills |
| PlayerControlBar chips | 16px | ⚠️ Trop rond |
| Avatars | 18px (cercle) | ✓ Correct |

### Système de Radius Recommandé

```javascript
var radiusNone = 0       // Bords à bord
var radiusSmall = 4      // Petits éléments (badges)
var radiusMedium = 8     // Standard (cartes, inputs)
var radiusLarge = 12     // Grandes cartes, modals
var radiusXLarge = 16    // Hero sections
var radiusFull = 9999    // Pills, avatars
```

**Le `cornerRadius: 22` global doit être supprimé ou réduit à 12.**

---

## 5. Élévation et Ombres

### Problème: Absence Quasi Totale

L'application n'utilise presque pas d'ombres, ce qui crée une interface **plate** sans hiérarchie de profondeur.

**Seule ombre trouvée:**
```qml
// StreamCard - ombre au hover
Rectangle {
  id: cardShadow
  border.color: "#00000020"  // Presque invisible
  opacity: 0  // Invisible par défaut!
}
```

### Système d'Ombres Recommandé

```javascript
// Ombres diffuses pour mode sombre
var shadowSubtle = "0 1px 2px rgba(0,0,0,0.3)"
var shadowSmall = "0 2px 4px rgba(0,0,0,0.4)"
var shadowMedium = "0 4px 8px rgba(0,0,0,0.5)"
var shadowLarge = "0 8px 16px rgba(0,0,0,0.6)"
var shadowXLarge = "0 16px 32px rgba(0,0,0,0.7)"
```

**Note:** QML ne supporte pas nativement box-shadow. Alternatives:
1. `DropShadow` effect (coûteux en performance)
2. Rectangle flou derrière l'élément
3. Gradient subtil en bordure

---

## 6. Analyse des Composants

### 6.1 StreamCard - Bon Base, Améliorations Possibles

**Points positifs:**
- Hover avec scale (1.02) - subtil et élégant ✓
- Transition douce (200ms OutCubic) ✓
- Badge LIVE bien positionné ✓
- Chargement async des images ✓

**Problèmes:**

| Problème | Détail |
|----------|--------|
| Couleurs hover hardcodées | `#252d3d`, `#1a2230` au lieu du thème |
| Hiérarchie typographique faible | 14/12/11px trop proches |
| Ombre invisible | `opacity: 0` par défaut, `0.3` au hover |
| Espacement interne | `margins: 12` devrait être dans le thème |

**Recommandations visuelles:**
- Augmenter le contraste entre titre (16px bold) et description (13px regular)
- Ajouter une ombre subtile permanente, plus prononcée au hover
- Utiliser l'accent color pour le nombre de viewers

### 6.2 PlayerControlBar - Surcharge Visuelle

**Problèmes majeurs:**

1. **Trop d'éléments au même niveau hiérarchique**
   - Play/Pause, Seek, Live, Time, Speed-, Speed, Speed+, HW, Crop, Chat, Volume, Fullscreen
   - 12 éléments tous de même importance visuelle

2. **Chips identiques partout**
   - Même taille (32x32), même couleur, même radius
   - L'utilisateur ne sait pas quoi regarder en premier

3. **Couleurs des contrôles**
   ```qml
   color: "#1AFFFFFF"  // 10% blanc - trop subtil
   color: "#33FFFFFF"  // 20% blanc - hover
   border.color: "#4DFFFFFF"  // 30% blanc
   ```
   - Faible contraste sur fond noir

**Recommandations:**

```
Hiérarchie visuelle recommandée:

[PRINCIPAL] Play/Pause - Plus grand, plus visible
[═══════════════════════════════════════] Seek slider
[SECONDAIRE] Live • 00:00 / 00:00
[TERTIAIRE] Speed | HW | Crop | Chat | Volume | ⛶

Grouper visuellement:
- Contrôles de lecture (gauche)
- Seek (centre, dominant)
- Options (droite, moins proéminentes)
```

### 6.3 ChatPanel - Style iOS Dépareillé

**Problème principal:** Le chat utilise des couleurs iOS (`#1C1C1E`, `#2C2C2E`, `#3A3A3C`) qui ne correspondent pas au BlueTheme.

**Palette Chat actuelle vs BlueTheme:**

| Élément | Chat (iOS) | BlueTheme |
|---------|------------|-----------|
| Background | `#1C1C1E` | `#141c2a` |
| Surface | `#2C2C2E` | `#1d2533` |
| Divider | `#3A3A3C` | `#222b37` |
| Accent | `#9147FF` (Twitch) | `#5bc0ff` |

**Recommandation:** Migrer vers les couleurs BlueTheme pour la cohérence, ou créer une section dédiée dans le thème si le style iOS est intentionnel.

### 6.4 LoginView - Bonne Exécution

**Points positifs:**
- Hiérarchie claire: Logo (56px) → Tagline (16px) → Card → Button
- Centrage vertical avec offset (-50) - respire
- Bouton accent bien visible
- Espacement généreux (`spacingLarge * 4`)

**Problèmes mineurs:**
- Couleur du texte bouton hardcodée (`#03050b`)
- Card background hardcodé (`#1b2130`, `#2a324e`)

### 6.5 SearchResults - Navigation au Clavier

**Points positifs:**
- Navigation clavier (↑↓ Enter) ✓
- Sections visuellement distinctes ✓
- Badges LIVE et CACHE différenciés ✓

**Problèmes:**
- Couleur hover hardcodée (`#1a2230`)
- Textes français non traduits ("En direct", "Dans le cache")

---

## 7. États des Composants

### Analyse des États Interactifs

| Composant | Default | Hover | Pressed | Focus | Disabled |
|-----------|---------|-------|---------|-------|----------|
| BlueButton | ✓ | ✗ | ✗ | ✗ | ✓ |
| StreamCard | ✓ | ✓ | ✗ | ✗ | N/A |
| ControlButton | ✓ | ✓ | ✓ | ✗ | ✗ |
| TextField | ✓ | ✓ | ✓ | ✓ | ✗ |
| Slider | ✓ | ✓ | ✓ | ✗ | ✓ |

### États Manquants

1. **Focus visible** - Critique pour accessibilité
   - Aucun composant n'a de ring de focus visible
   - Navigation clavier impossible à suivre visuellement

2. **Pressed/Active**
   - Les boutons n'ont pas d'état pressed distinct
   - Le feedback tactile est absent

3. **Loading**
   - Pas de skeletons pour les cartes en chargement
   - Juste des emojis (⋯, ⏳, 📺)

### Recommandations États

```qml
// État Focus - À ajouter sur tous les éléments interactifs
Rectangle {
  visible: parent.activeFocus
  anchors.fill: parent
  anchors.margins: -2
  color: "transparent"
  border.color: BlueTheme.accent
  border.width: 2
  radius: parent.radius + 2
}

// État Pressed
scale: pressed ? 0.95 : (hovered ? 1.02 : 1.0)
opacity: pressed ? 0.8 : 1.0
```

---

## 8. Animations et Transitions

### Analyse Actuelle

| Animation | Durée | Easing | Verdict |
|-----------|-------|--------|---------|
| Card hover | 200ms | OutCubic | ✓ Fluide |
| Control bar fade | 300ms | InOutCubic | ✓ Bon |
| Color transitions | 150-200ms | Linear/OutCubic | ⚠️ Incohérent |
| Volume popup | 200ms | OutCubic | ✓ Bon |
| Center feedback | 100+300+250ms | - | ✓ Satisfaisant |

### Problèmes

1. **Durées incohérentes** - 100, 150, 180, 200, 220, 250, 300ms
2. **Easing incohérent** - Linear, OutCubic, InOutCubic mélangés
3. **Pas de transitions entre vues** - Changement abrupt Home → Player

### Système d'Animation Recommandé

```javascript
// Durées
var durationFast = 100      // Micro-interactions
var durationNormal = 200    // Standard
var durationSlow = 300      // Transitions importantes
var durationEnter = 250     // Entrée de composant
var durationExit = 200      // Sortie de composant

// Easings
var easingStandard = Easing.OutCubic
var easingEnter = Easing.OutQuart
var easingExit = Easing.InCubic
var easingBounce = Easing.OutBack
```

---

## 9. Accessibilité Visuelle

### Problèmes Critiques

| Problème | Sévérité | Localisation |
|----------|----------|--------------|
| Pas de focus visible | Critique | Tous les composants |
| Contraste mutedText | Élevée | Thème global |
| Texte trop petit | Moyenne | 10-11px dans métadonnées |
| Pas de mode contraste élevé | Basse | Global |

### Recommandations WCAG

1. **Contraste minimum 4.5:1** pour tout texte
2. **Focus visible** sur tous les éléments interactifs
3. **Taille minimum 12px** pour texte lisible
4. **Zones tactiles 44x44px** minimum

---

## 10. Cohérence Globale

### Inventaire des Incohérences

| Élément | Variations trouvées |
|---------|---------------------|
| Corner radius | 8, 10, 12, 14, 16, 18, 22, 30px |
| Button height | 32, 34, 38, 44, 50px |
| Padding interne | 8, 12, 14, 16, 20, 24px |
| Font sizes | 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 56px |
| Hover colors | 6 variantes différentes |

### Score de Cohérence: 5/10

L'application manque d'un **design system documenté et appliqué rigoureusement**.

---

## Sous-tâches de Design

### Sprint A - Fondations du Design System
- **25.1** - Compléter la palette de couleurs BlueTheme
- **25.2** - Définir l'échelle typographique
- **25.3** - Standardiser le système de radius (4/8/12/16)
- **25.4** - Définir le système d'ombres

### Sprint B - Composants de Base
- **25.5** - Créer BaseButton avec tous les états
- **25.6** - Créer FocusRing réutilisable
- **25.7** - Créer SkeletonLoader pour le chargement
- **25.8** - Unifier le style des Cards (BaseCard)

### Sprint C - Cohérence Visuelle
- **25.9** - Migrer ChatPanel vers BlueTheme
- **25.10** - Migrer PlayerControlBar vers BlueTheme
- **25.11** - Supprimer toutes les couleurs hardcodées
- **25.12** - Harmoniser les hover states

### Sprint D - Hiérarchie & Animation
- **25.13** - Refondre la hiérarchie de PlayerControlBar
- **25.14** - Ajouter transitions entre vues
- **25.15** - Standardiser les durées/easings d'animation
- **25.16** - Ajouter feedback pressed sur tous les boutons

### Sprint E - Accessibilité
- **25.17** - Implémenter focus visible partout
- **25.18** - Corriger les contrastes insuffisants
- **25.19** - Augmenter les tailles de texte minimum
- **25.20** - Documenter les raccourcis clavier dans l'UI

---

## Priorité des Sous-tâches

| Priorité | Sous-tâche | Effort | Impact Visuel |
|----------|------------|--------|---------------|
| 1 | 25.1 | S | Élevé |
| 1 | 25.3 | S | Élevé |
| 1 | 25.11 | M | Élevé |
| 2 | 25.2 | S | Moyen |
| 2 | 25.8 | M | Élevé |
| 2 | 25.9 | M | Élevé |
| 2 | 25.10 | M | Élevé |
| 3 | 25.5 | S | Moyen |
| 3 | 25.6 | S | Moyen |
| 3 | 25.12 | M | Moyen |
| 3 | 25.17 | M | Critique (a11y) |
| 4 | 25.4 | S | Moyen |
| 4 | 25.7 | S | Moyen |
| 4 | 25.13 | L | Élevé |
| 5 | 25.14 | M | Moyen |
| 5 | 25.15 | S | Faible |
| 5 | 25.16 | S | Faible |
| 5 | 25.18 | S | Critique (a11y) |
| 5 | 25.19 | S | Moyen |
| 5 | 25.20 | S | Faible |

---

## BlueTheme.js - Version Recommandée

```javascript
.pragma library

// ============================================
// COULEURS
// ============================================

// Backgrounds
var windowBackground = "#03050b"
var gradientStart = "#040b15"
var gradientEnd = "#0b1727"

// Surfaces
var surface = "#141c2a"
var surfaceElevated = "#1a2332"
var surfaceHover = "#1e2840"
var surfaceSoft = "#1d2533"
var overlayTint = "#0c111b"

// Player (noir pur pour vidéo)
var playerBackground = "#000000"
var playerSurface = "rgba(0,0,0,0.8)"
var playerControlBg = "rgba(255,255,255,0.1)"
var playerControlHover = "rgba(255,255,255,0.2)"
var playerControlBorder = "rgba(255,255,255,0.3)"

// Accents
var accent = "#5bc0ff"
var accentSubtle = "#3da2ff"
var accentMuted = "#2a7ab8"

// Texte
var primaryText = "#f6f7fa"
var secondaryText = "#aeb9c9"
var mutedText = "#8a96b0"  // Ajusté pour contraste 5:1
var disabledText = "#5a6578"

// Bordures & Dividers
var divider = "#222b37"
var border = "#2a3444"
var borderFocus = accent

// Statuts
var statusPositive = "#4ef57a"
var statusWarning = "#f7c114"
var statusNegative = "#f46969"
var statusLive = "#f46969"

// Buttons
var buttonSurface = "#1c2232"
var buttonBorder = "#2b3450"
var buttonHover = "#252d40"
var buttonPressed = "#1a222f"

// Cards
var cardBackground = surface
var cardHover = "#1a2330"
var cardBorder = divider
var cardHighlight = "#171f2f"

// ============================================
// TYPOGRAPHIE
// ============================================

function selectFontFamily() {
  var preferredFonts = ["SF Pro Display", "Helvetica Neue", "Inter", "Arial", "Segoe UI"];
  var families = Qt.fontFamilies();
  for (var idx = 0; idx < preferredFonts.length; ++idx) {
    if (families.indexOf(preferredFonts[idx]) !== -1) return preferredFonts[idx];
  }
  return families.length > 0 ? families[0] : "Sans Serif";
}

var fontFamily = selectFontFamily()
var headlineFont = fontFamily

// Échelle typographique
var fontSizeHero = 48
var fontSizeH1 = 24
var fontSizeH2 = 20
var fontSizeH3 = 16
var fontSizeBody = 14
var fontSizeCaption = 12
var fontSizeSmall = 11

// Poids
var fontWeightRegular = 400
var fontWeightMedium = 500
var fontWeightSemibold = 600
var fontWeightBold = 700

// Line-height
var lineHeightTight = 1.2
var lineHeightNormal = 1.4
var lineHeightRelaxed = 1.6

// ============================================
// ESPACEMENT (Base 8)
// ============================================

var spacing4 = 4
var spacing8 = 8
var spacing12 = 12
var spacing16 = 16
var spacing24 = 24
var spacing32 = 32
var spacing48 = 48
var spacing64 = 64

// Aliases
var spacingSmall = spacing8
var spacingMedium = spacing16
var spacingLarge = spacing24

// ============================================
// RADIUS
// ============================================

var radiusSmall = 4
var radiusMedium = 8
var radiusLarge = 12
var radiusXLarge = 16
var radiusFull = 9999

// Aliases
var cornerRadius = radiusLarge  // Réduit de 22 à 12
var heroCornerRadius = radiusXLarge
var buttonRadius = radiusMedium
var cardRadius = radiusLarge
var badgeRadius = radiusFull

// ============================================
// ÉLÉVATION
// ============================================

var elevation = 28
var cardElevation = 10
var borderWidth = 1

// ============================================
// DIMENSIONS
// ============================================

var responsiveBreakpoint = 1024
var heroHeight = 260

// Boutons
var buttonHeightSmall = 32
var buttonHeightMedium = 40
var buttonHeightLarge = 50

// Touch targets
var touchTargetMin = 44

// ============================================
// ANIMATION
// ============================================

var durationFast = 100
var durationNormal = 200
var durationSlow = 300
```

---

## Checklist de Validation Design

### Palette & Thème
- [ ] Toutes les couleurs dans BlueTheme.js
- [ ] Aucune couleur hardcodée dans les .qml
- [ ] Contraste WCAG AA respecté (4.5:1)
- [ ] Style visuel unifié (plus de mélange iOS/Blue)

### Typographie
- [ ] Échelle typographique appliquée
- [ ] Taille minimum 11px respectée
- [ ] Hiérarchie visible (titres vs corps)

### Espacement
- [ ] Toutes les valeurs multiples de 4 ou 8
- [ ] Système de spacing utilisé partout
- [ ] Padding/margin cohérents

### Composants
- [ ] Tous les états définis (default, hover, pressed, focus, disabled)
- [ ] Focus visible sur éléments interactifs
- [ ] Hover feedback sur éléments cliquables
- [ ] Skeletons pour chargement

### Animation
- [ ] Durées standardisées
- [ ] Easings cohérents
- [ ] Transitions entre vues

---

## Métriques Cibles Design

| Métrique | Actuel | Cible |
|----------|--------|-------|
| Couleurs dans thème | ~20 | 40+ |
| Couleurs hardcodées | ~40 | 0 |
| Radius variants | 10+ | 5 |
| Composants avec focus visible | 0% | 100% |
| Score cohérence | 5/10 | 9/10 |

---

## Notes Finales

Ce plan est le résultat d'un **audit UI/UX approfondi** avec un œil de designer.

**Philosophie à adopter:**
- L'espace blanc est ton ami
- La cohérence bat l'originalité
- Simple n'est pas ennuyeux, simple est élégant
- Chaque pixel doit avoir une raison d'être

**Estimation:** 30-40 heures de travail design
