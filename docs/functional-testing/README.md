# Tests Fonctionnels UI - BluePlayer

Ce dossier contient la documentation complète pour les tests fonctionnels de l'interface utilisateur de BluePlayer.

## Vue d'ensemble

Les tests fonctionnels UI vérifient que les interactions utilisateur (clics, drags, saisie clavier) produisent les comportements attendus dans l'interface. Contrairement aux tests unitaires qui testent la logique métier, les tests fonctionnels valident l'expérience utilisateur de bout en bout.

## Stack technique

| Technologie | Rôle |
|-------------|------|
| **Qt Quick Test** | Framework de test pour QML (natif Qt 6) |
| **QML TestCase** | Type de base pour écrire les tests |
| **SignalSpy** | Capture et vérifie les signaux émis |
| **Qt Test** | Intégration avec CTest |

## Structure des fichiers

```
tests/functional/
├── main.cpp                    # Point d'entrée C++
├── tst_PlayerControlBar.qml    # Tests du PlayerControlBar
├── tst_VideoPlayer.qml         # Tests du VideoPlayer (à venir)
├── tst_HomeView.qml            # Tests du HomeView (à venir)
└── helpers/                    # Utilitaires partagés (à venir)
    └── TestUtils.qml

docs/functional-testing/
├── README.md                   # Ce fichier
├── SCENARIOS.md                # Catalogue des scénarios de test
├── PATTERNS.md                 # Patterns et bonnes pratiques
└── SETUP.md                    # Guide d'installation et configuration
```

## Documentation

| Document | Description |
|----------|-------------|
| [SETUP.md](./SETUP.md) | Installation, configuration, exécution des tests |
| [SCENARIOS.md](./SCENARIOS.md) | Catalogue complet des scénarios de test par composant |
| [PATTERNS.md](./PATTERNS.md) | Patterns de test, bonnes pratiques, anti-patterns |

## Démarrage rapide

### Prérequis

- Qt 6.5+ avec le module `QuickTest`
- CMake 3.24+
- Build existant de BluePlayer

### Exécution des tests

```bash
# Build avec les tests
cmake -B build -DBLUEPLAYER_ENABLE_TESTS=ON
cmake --build build

# Exécuter tous les tests fonctionnels
./build/tests/test_functional_ui

# Lister les tests disponibles
./build/tests/test_functional_ui -functions

# Exécuter un test spécifique
./build/tests/test_functional_ui PlayerControlBarTests::test_playPauseButton_click_emitsSignal

# Mode verbose
./build/tests/test_functional_ui -v2
```

## Philosophie de test

### Ce que nous testons

1. **Interactions utilisateur** → Clics, drags, hovers, saisie clavier
2. **Émission de signaux** → Les actions déclenchent les bons signaux
3. **Changements d'état visuel** → L'UI reflète correctement l'état
4. **Flux de navigation** → Les transitions entre vues fonctionnent

### Ce que nous NE testons PAS ici

1. ❌ Logique métier (→ tests unitaires C++)
2. ❌ Appels API (→ tests d'intégration)
3. ❌ Performance (→ benchmarks dédiés)
4. ❌ Rendu pixel-perfect (→ tests visuels/snapshots)

## Conventions de nommage

### Fichiers de test

```
tst_<ComponentName>.qml
```

Exemples : `tst_PlayerControlBar.qml`, `tst_HomeView.qml`

### Fonctions de test

```qml
function test_<element>_<action>_<expectedResult>() { }
```

Exemples :
- `test_playButton_click_emitsPlayPauseSignal()`
- `test_volumeSlider_drag_updatesVolume()`
- `test_searchInput_enterKey_triggersSearch()`

## Métriques cibles

| Métrique | Cible |
|----------|-------|
| Couverture des composants critiques | 100% |
| Tests par composant UI majeur | ≥ 10 |
| Temps d'exécution total | < 30s |
| Tests flaky | 0 |

## Roadmap

- [x] Infrastructure de base (CMake, main.cpp)
- [x] POC avec PlayerControlBar
- [ ] Tests complets PlayerControlBar (composant réel)
- [ ] Tests HomeView
- [ ] Tests VideoPlayer
- [ ] Tests LoginView
- [ ] Tests de navigation inter-vues
- [ ] Helpers et utilitaires partagés
- [ ] Intégration CI/CD
