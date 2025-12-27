# Plan 24 - Refactoring Profond & Corrections Critiques

## Contexte

Suite à une analyse approfondie de la codebase BluePlayer (27 décembre 2025), plusieurs problèmes critiques et opportunités d'amélioration ont été identifiés. Ce plan regroupe toutes les tâches de refactoring et corrections issues de cette analyse.

**Analyse effectuée sur :**
- Backend C++ (~7,500 lignes)
- Interface QML (22 fichiers, ~6,000 lignes)
- Tests et configuration qualité
- Architecture globale

---

## Scores de l'Analyse

| Dimension | Score | Verdict |
|-----------|-------|---------|
| Architecture C++ | 7/10 | Solide mais dettes techniques |
| Interface QML | 6.5/10 | Fichiers trop longs, valeurs hardcodées |
| Tests & Qualité | 7.5/10 | Bonne base, gaps E2E |
| Sécurité | 4/10 | **CRITIQUE** |
| Maintenabilité | 6/10 | Duplication significative |

---

## Sous-tâches

### Sprint 1 - Sécurité & Stabilité (CRITIQUE)
- **24.1** - Remplacer chiffrement XOR par macOS Keychain
- **24.2** - Supprimer tous les debug logs hardcodés
- **24.3** - Corriger Logger::error (qCWarning → qCCritical)
- **24.4** - Ajouter validation messages chat IRC
- **24.5** - Rendre bools atomiques dans MpvQuickItem

### Sprint 2 - Refactoring QML PlayerView
- **24.6** - Extraire TopBarOverlay.qml depuis PlayerView
- **24.7** - Extraire LoadingOverlay.qml depuis PlayerView
- **24.8** - Extraire ErrorToast.qml depuis PlayerView
- **24.9** - Déplacer logique recording vers backend C++

### Sprint 3 - Refactoring QML PlayerControlBar
- **24.10** - Créer ControlButton.qml (composant réutilisable)
- **24.11** - Extraire VolumeControl.qml
- **24.12** - Extraire SeekBar.qml
- **24.13** - Remplacer Canvas icons par SVG/icon font

### Sprint 4 - Refactoring QML Cards
- **24.14** - Créer BaseCard.qml avec hover/shadow/placeholder
- **24.15** - Refactorer 5 cards pour étendre BaseCard

### Sprint 5 - Refactoring C++ TwitchService
- **24.16** - Extraire helper setTokenAndRefresh()
- **24.17** - Créer TwitchStreamService (streams, recommended)
- **24.18** - Créer TwitchContentService (clips, videos, channels)
- **24.19** - Créer TwitchSearchService (search, categories)

### Sprint 6 - Refactoring C++ Media
- **24.20** - Créer MpvPlayerBase.cpp pour init partagée
- **24.21** - Rendre downloadThumbnail() asynchrone

### Sprint 7 - Expansion Thème
- **24.22** - Ajouter 15+ couleurs manquantes dans BlueTheme.js
- **24.23** - Migrer toutes les valeurs hardcodées vers le thème

### Sprint 8 - Accessibilité
- **24.24** - Ajouter Accessible.role sur tous les boutons
- **24.25** - Ajouter Accessible.name descriptifs
- **24.26** - Ajouter indicateurs de focus visibles

### Sprint 9 - Tests & CI
- **24.27** - Créer TestApplication.cpp
- **24.28** - Ajouter 3-5 tests d'intégration
- **24.29** - Configurer GitHub Actions CI/CD
- **24.30** - Créer 2-3 tests E2E basiques

---

## Priorité des sous-tâches

| Priorité | Sous-tâche | Effort | Impact | Dépendances |
|----------|------------|--------|--------|-------------|
| **CRITIQUE** | 24.1 | M | Sécurité | Aucune |
| **CRITIQUE** | 24.2 | S | Build/Sécurité | Aucune |
| **CRITIQUE** | 24.3 | S | Logging | Aucune |
| 1 | 24.4 | S | Sécurité | Aucune |
| 1 | 24.5 | S | Stabilité | Aucune |
| 2 | 24.10 | S | Maintenabilité | Aucune |
| 2 | 24.14 | M | Maintenabilité | Aucune |
| 2 | 24.22 | S | Cohérence | Aucune |
| 3 | 24.6 | S | Maintenabilité | Aucune |
| 3 | 24.7 | S | Maintenabilité | Aucune |
| 3 | 24.8 | S | Maintenabilité | Aucune |
| 3 | 24.11 | M | Maintenabilité | 24.10 |
| 3 | 24.12 | M | Maintenabilité | 24.10 |
| 4 | 24.15 | M | Maintenabilité | 24.14 |
| 4 | 24.16 | S | DRY | Aucune |
| 4 | 24.23 | M | Cohérence | 24.22 |
| 5 | 24.9 | L | Architecture | 24.6-24.8 |
| 5 | 24.13 | M | Performance | 24.11, 24.12 |
| 5 | 24.17 | L | Architecture | 24.16 |
| 5 | 24.18 | L | Architecture | 24.16 |
| 5 | 24.19 | L | Architecture | 24.16 |
| 6 | 24.20 | M | DRY | Aucune |
| 6 | 24.21 | M | Performance | Aucune |
| 7 | 24.24 | S | Accessibilité | Aucune |
| 7 | 24.25 | S | Accessibilité | 24.24 |
| 7 | 24.26 | M | Accessibilité | 24.24 |
| 8 | 24.27 | M | Tests | Aucune |
| 8 | 24.28 | L | Tests | 24.27 |
| 8 | 24.29 | M | CI/CD | Aucune |
| 8 | 24.30 | L | Tests | 24.28 |

**Légende effort:** S = Small (<1h), M = Medium (1-4h), L = Large (4h+)

---

## Détails des Sous-tâches

### 24.1 - Remplacer chiffrement XOR par macOS Keychain

**Fichier:** `src/core/SecureStorage.cpp:89-125`

**Problème actuel:**
```cpp
// DANGEREUX - XOR n'est PAS du chiffrement
for (int i = 0; i < data.size(); ++i) {
    encrypted.append(data[i] ^ key[i % key.size()]);
}
```

**Solution:**
- Utiliser `Security.framework` (macOS Keychain)
- Alternative: libsodium pour chiffrement AES-256-GCM
- Stocker tokens OAuth de manière sécurisée

**Checklist:**
- [ ] Implémenter wrapper Keychain
- [ ] Migrer stockage existant
- [ ] Supprimer code XOR
- [ ] Tester sur credentials réels

---

### 24.2 - Supprimer debug logs hardcodés

**Fichier:** `src/api/twitch/TwitchAuthManager.cpp`

**Lignes affectées:** ~222-311, 448-466, 505-521, 545-561, 564-578, 818-835, 896-914, 923-936, 948-960

**Pattern à supprimer:**
```cpp
// #region agent log
QFile logFile(QStringLiteral("/Users/user/Projects/BluePlayer/.cursor/debug.log"));
// ... tout le bloc
// #endregion
```

**Commande:**
```bash
grep -rn "agent log" src/
# Supprimer tous les blocs #region agent log ... #endregion
```

**Checklist:**
- [ ] Identifier tous les fichiers concernés
- [ ] Supprimer les blocs agent log
- [ ] Vérifier compilation
- [ ] Vérifier fonctionnement auth

---

### 24.3 - Corriger Logger::error

**Fichier:** `src/core/Logger.cpp:105-107`

**Problème:**
```cpp
void Logger::error(const QLoggingCategory& category, const QString& message) {
  qCWarning(category).noquote() << "[ERROR]" << message;  // BUG!
}
```

**Solution:**
```cpp
void Logger::error(const QLoggingCategory& category, const QString& message) {
  qCCritical(category).noquote() << message;
}
```

---

### 24.4 - Validation messages chat IRC

**Fichier:** `src/chat/TwitchChatClient.cpp:71-95`

**Problème:** Pas de validation du contenu des messages (injection IRC possible)

**Solution:**
```cpp
void TwitchChatClient::sendMessage(const QString& message) {
    // Valider et sanitizer le message
    QString sanitized = message;
    sanitized.remove('\r');
    sanitized.remove('\n');
    sanitized = sanitized.trimmed();
    
    if (sanitized.isEmpty() || sanitized.length() > 500) {
        return;
    }
    
    QString cmd = QStringLiteral("PRIVMSG #%1 :%2").arg(m_channel, sanitized);
    // ...
}
```

---

### 24.5 - Bools atomiques MpvQuickItem

**Fichier:** `src/media/MpvQuickItem.hpp:143-144`

**Problème:**
```cpp
bool m_userInitiatedSeek{false};      // Non thread-safe
bool m_wasAtLiveEdgeBeforePause{false}; // Non thread-safe
```

**Solution:**
```cpp
std::atomic<bool> m_userInitiatedSeek{false};
std::atomic<bool> m_wasAtLiveEdgeBeforePause{false};
```

---

### 24.6 à 24.9 - Refactoring PlayerView.qml

**Fichier:** `src/ui/PlayerView.qml` (994 lignes → ~400 lignes cible)

**Extractions:**

| Nouveau fichier | Lignes source | Contenu |
|-----------------|---------------|---------|
| TopBarOverlay.qml | 478-571 | Gradient top, back button, stream info |
| LoadingOverlay.qml | 617-634 | Rectangle + BusyIndicator |
| ErrorToast.qml | 805-824 | Rectangle erreur avec timer |

**24.9 - Recording vers C++:**
- Déplacer `startAutoRecording()` et `saveRecordingIfNeeded()` vers `CacheManager`
- Exposer via Q_INVOKABLE
- Simplifier QML en appelant le service

---

### 24.10 à 24.13 - Refactoring PlayerControlBar.qml

**Fichier:** `src/ui/components/PlayerControlBar.qml` (789 lignes → ~250 lignes cible)

**24.10 - ControlButton.qml:**
```qml
// Nouveau composant réutilisable
Rectangle {
    id: controlButton
    property string icon: ""
    property string tooltipText: ""
    property bool active: false
    signal clicked()
    
    width: 32; height: 32; radius: 16
    color: mouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
    border.color: "#4DFFFFFF"
    // ...
}
```

**24.11 - VolumeControl.qml:** ~180 lignes (529-706)
**24.12 - SeekBar.qml:** ~130 lignes (161-280)

---

### 24.14 à 24.15 - BaseCard.qml

**Fichiers concernés:**
- StreamCard.qml
- VideoCard.qml
- ClipCard.qml
- ChannelCard.qml
- CategoryCard.qml

**BaseCard.qml (nouveau):**
```qml
Item {
    id: baseCard
    property bool isPlaceholder: false
    property alias contentItem: contentLoader.sourceComponent
    
    // Hover state partagé (~30 lignes)
    // Shadow partagée (~10 lignes)
    // Placeholder partagé (~15 lignes)
    
    Rectangle {
        id: cardBackground
        // ...
        states: [ State { name: "hovered" ... } ]
        transitions: [ Transition { ... } ]
        
        Loader { id: contentLoader }
    }
    
    MouseArea { id: mouseArea ... }
}
```

---

### 24.16 à 24.19 - Refactoring TwitchService

**Fichier:** `src/api/twitch/TwitchService.cpp` (1125 lignes)

**24.16 - Helper token:**
```cpp
// Nouveau helper privé
void TwitchService::ensureTokenAndExecute(std::function<void()> action) {
    QString token = m_authManager ? m_authManager->accessToken() : QString();
    if (!token.isEmpty()) {
        m_apiClient->setAccessToken(token);
        action();
    }
}

// Usage (remplace 15 duplications)
void TwitchService::refreshStreams() {
    ensureTokenAndExecute([this]() {
        m_apiClient->getFollowedStreams(m_userId);
    });
}
```

**24.17-19 - Nouveaux services:**

| Service | Responsabilités | Méthodes |
|---------|-----------------|----------|
| TwitchStreamService | Streams live | refreshStreams, refreshRecommended, refreshCategoryStreams |
| TwitchContentService | Contenu statique | refreshClips, refreshVideos, refreshChannels |
| TwitchSearchService | Recherche | search, clearSearch |

---

### 24.22 à 24.23 - Expansion BlueTheme.js

**Fichier:** `src/ui/themes/BlueTheme.js`

**Couleurs à ajouter:**
```javascript
// Player
var playerBackground = "#000000"
var semiTransparentDark = "#CC000000"
var semiTransparentMedium = "#80000000"
var semiTransparentLight = "#4D000000"

// Controls
var controlButtonBg = "#1AFFFFFF"
var controlButtonHover = "#33FFFFFF"
var controlButtonBorder = "#4DFFFFFF"

// Chat
var chatBackground = "#1C1C1E"
var chatSurface = "#2C2C2E"
var chatDivider = "#3A3A3C"

// Search
var searchBarBg = "#0d1117"
var searchBarFocusBg = "#161d28"

// Cards
var hoverTint = "#1a2230"
var hoverTintLight = "#252d3d"
```

---

## Fichiers Concernés (Résumé)

### C++ (Priorité haute)
- `src/core/SecureStorage.cpp` - Sécurité critique
- `src/core/Logger.cpp` - Bug logging
- `src/api/twitch/TwitchAuthManager.cpp` - Debug logs
- `src/api/twitch/TwitchService.cpp` - God class
- `src/media/MpvQuickItem.hpp/cpp` - Thread safety
- `src/chat/TwitchChatClient.cpp` - Validation

### QML (Priorité haute)
- `src/ui/PlayerView.qml` - 994 lignes
- `src/ui/components/PlayerControlBar.qml` - 789 lignes

### QML (Priorité moyenne)
- `src/ui/CacheManagerView.qml` - 735 lignes
- `src/ui/HomeView.qml` - 496 lignes
- `src/ui/components/StreamCard.qml` - Duplication
- `src/ui/components/VideoCard.qml` - Duplication
- `src/ui/components/ClipCard.qml` - Duplication
- `src/ui/components/ChannelCard.qml` - Duplication
- `src/ui/components/CategoryCard.qml` - Duplication
- `src/ui/themes/BlueTheme.js` - Extension

---

## Métriques Cibles

| Métrique | Actuel | Cible | Amélioration |
|----------|--------|-------|--------------|
| PlayerView.qml | 994 lignes | < 400 lignes | -60% |
| PlayerControlBar.qml | 789 lignes | < 250 lignes | -68% |
| TwitchService.cpp | 1125 lignes | < 400 lignes | -65% |
| Valeurs hardcodées QML | ~40 | 0 | -100% |
| Duplication Cards | ~250 lignes | ~50 lignes | -80% |
| Score sécurité | 4/10 | 9/10 | +125% |

---

## Checklist de Validation Globale

### Sécurité (Sprint 1)
- [ ] 24.1 - Keychain implémenté et testé
- [ ] 24.2 - Tous les debug logs supprimés
- [ ] 24.3 - Logger::error corrigé
- [ ] 24.4 - Validation chat implémentée
- [ ] 24.5 - Bools atomiques

### Refactoring QML (Sprints 2-4)
- [ ] 24.6-24.9 - PlayerView refactoré
- [ ] 24.10-24.13 - PlayerControlBar refactoré
- [ ] 24.14-24.15 - BaseCard créé et utilisé

### Refactoring C++ (Sprints 5-6)
- [ ] 24.16-24.19 - TwitchService splitté
- [ ] 24.20-24.21 - Media refactoré

### Thème & Accessibilité (Sprints 7-8)
- [ ] 24.22-24.23 - BlueTheme complété
- [ ] 24.24-24.26 - Accessibilité ajoutée

### Tests (Sprint 9)
- [ ] 24.27-24.30 - Tests et CI ajoutés

### Qualité Finale
- [ ] Tous les tests passent
- [ ] Build sans warnings
- [ ] Pas de régression fonctionnelle
- [ ] Code review effectuée

---

## Notes

Ce plan est le résultat d'une **analyse approfondie** de la codebase.

**Priorité absolue:** Les tâches 24.1 à 24.3 sont **CRITIQUES** et doivent être traitées avant toute autre chose.

**Ordre recommandé:**
1. Sprint 1 (Sécurité) - Immédiat
2. Sprints 2-4 (QML) - Court terme
3. Sprints 5-6 (C++) - Moyen terme
4. Sprints 7-9 (Polish) - Long terme

**Estimation totale:** 40-60 heures de travail
