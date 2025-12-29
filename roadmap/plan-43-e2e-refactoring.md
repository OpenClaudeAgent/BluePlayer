# Plan 43 - Refactoring Infrastructure E2E

## Contexte

L'infrastructure de tests E2E a ete implementee dans le Plan 40 (v0.29.0) et comprend :
- MockTwitchServer et MockHlsServer pour simuler les services Twitch
- Deux contexts (AuthenticatedSetup, UnauthenticatedSetup)
- Deux scenarios (open-stream, login-view) totalisant 13 tests
- E2ETestCase.qml comme classe de base pour les tests QML

Bien que fonctionnelle, cette infrastructure presente plusieurs opportunites d'amelioration en termes de maintenabilite, reutilisabilite et extensibilite.

### Problemes identifies

#### 1. Duplication massive entre les contexts (~80%)
Les fichiers `AuthenticatedSetup.cpp` et `UnauthenticatedSetup.cpp` partagent la quasi-totalite de leur code :
- Demarrage des mock servers (identique)
- Chargement des fixtures (similaire)
- Configuration de l'environnement (identique)
- Registration des types QML (identique)
- Configuration des import paths (identique)

La seule difference : l'un injecte les credentials, l'autre non.

#### 2. Mock Servers sans classe de base commune
`MockTwitchServer` et `MockHlsServer` dupliquent :
- La logique de serveur TCP (`handleNewConnection`, `handleClientDisconnected`)
- Le parsing HTTP (`parseRequest`)
- La construction des reponses HTTP (`makeResponse`)
- La gestion du lifecycle (`start`, `stop`, `isRunning`)

#### 3. Gestion fragile des fixtures
Le chemin des fixtures est calcule avec 3 heuristiques successives :
```cpp
QString fixturesPath = QDir::currentPath() + "/tests/e2e/fixtures";
if (!QDir(fixturesPath).exists()) {
    fixturesPath = QCoreApplication::applicationDirPath() + "/../../../../../tests/e2e/fixtures";
}
if (!QDir(fixturesPath).exists()) {
    fixturesPath = QCoreApplication::applicationDirPath() + "/../../fixtures";
}
```
Cette approche est fragile et non portable.

#### 4. Magic strings non centralisees
Les `objectName` utilises pour trouver les elements QML sont disperses :
- `"streamCard_"`, `"loginRoot"`, `"playerView"`, `"loginButton"`, `"mainWindow"`
- Les timeouts sont hardcodes (1000, 1500, 2000, 5000, 10000 ms)
- Les ports sont hardcodes (8080, 8081, 8082, 8083)

#### 5. Boilerplate dans les scenarios
Chaque scenario QML repete :
- Le wrapper `Item { Loader { ... } }`
- L'initialisation du `mainWindow`
- Les `wait()` pour la stabilisation

## Objectif

Refactorer l'infrastructure E2E pour :
1. **Eliminer la duplication** entre les contexts via une classe de base commune
2. **Unifier les mock servers** avec une classe `MockHttpServer` parente
3. **Centraliser la gestion des fixtures** via un `FixtureLoader` dedie
4. **Extraire les constantes** dans des fichiers de configuration
5. **Simplifier la creation de scenarios** avec un template standard

Le refactoring doit etre **non-breaking** : tous les tests existants doivent continuer a passer.

## Specifications

### Phase 1 : Refactoring des Contexts

#### 1.1 Creer BaseE2EContext
```
tests/e2e/contexts/
  BaseE2EContext.hpp        # Classe de base abstraite
  BaseE2EContext.cpp        # Implementation commune
  E2EContextHelpers.hpp     # Helpers existants (inchange)
  authenticated/            # Simplifie, herite de BaseE2EContext
  unauthenticated/          # Simplifie, herite de BaseE2EContext
```

**BaseE2EContext doit gerer :**
- Demarrage/arret des mock servers
- Chargement des fixtures (via FixtureLoader)
- Configuration de l'environnement
- Registration des types QML
- Configuration des import paths

**Les contexts derives ne font que :**
- Definir s'ils injectent des credentials ou non
- Eventuellement surcharger des comportements specifiques

#### 1.2 Estimation de reduction
- Avant : ~250 lignes x 2 contexts = ~500 lignes
- Apres : ~300 lignes (base) + 2 x 30 lignes (derives) = ~360 lignes
- **Reduction attendue : ~28%**

### Phase 2 : Refactoring des Mock Servers

#### 2.1 Creer MockHttpServer
```
tests/e2e/servers/
  MockHttpServer.hpp        # Classe de base avec logique TCP/HTTP
  MockHttpServer.cpp        # Implementation commune
  MockTwitchServer.hpp/cpp  # Simplifie, herite de MockHttpServer
  MockHlsServer.hpp/cpp     # Simplifie, herite de MockHttpServer
```

**MockHttpServer doit gerer :**
- Lifecycle du serveur TCP (`start`, `stop`, `isRunning`, `port`, `baseUrl`)
- Connexions clients (`handleNewConnection`, `handleClientDisconnected`)
- Parsing HTTP (`parseRequest`)
- Construction des reponses (`makeResponse`)
- Tracking des requetes (`requestCount`, `clearRequests`)

**Les servers derives ne font que :**
- Implementer `handleRequest()` pour leurs endpoints specifiques
- Definir leurs methodes de configuration (setStreams, addChannel, etc.)

#### 2.2 Estimation de reduction
- Avant : ~405 lignes (Twitch) + ~352 lignes (HLS) = ~757 lignes
- Apres : ~200 lignes (base) + 250 + 200 (derives) = ~650 lignes
- **Reduction attendue : ~14%**

### Phase 3 : FixtureLoader centralise

#### 3.1 Creer FixtureLoader
```
tests/e2e/fixtures/
  FixtureLoader.hpp         # Classe utilitaire pour charger les fixtures
  FixtureLoader.cpp
  auth_token.json           # Existant
  streams.json              # Existant
  users.json                # Existant
  test_segment.ts           # Existant
```

**FixtureLoader doit :**
- Determiner le chemin des fixtures de maniere robuste (variable d'env ou CMake)
- Charger et parser les fichiers JSON avec validation
- Charger les fichiers binaires (.ts)
- Emettre des erreurs claires si un fichier manque ou est invalide

#### 3.2 Configuration CMake
Ajouter une variable `E2E_FIXTURES_PATH` definie par CMake et passee via `target_compile_definitions`.

### Phase 4 : Centralisation des constantes

#### 4.1 Creer E2EConstants.hpp
```
tests/e2e/
  E2EConstants.hpp          # Toutes les constantes E2E
```

**Contenu :**
```cpp
namespace blueplayer::test::e2e::constants {
    // ObjectNames QML
    constexpr auto MAIN_WINDOW = "mainWindow";
    constexpr auto PLAYER_VIEW = "playerView";
    constexpr auto LOGIN_ROOT = "loginRoot";
    constexpr auto LOGIN_BUTTON = "loginButton";
    constexpr auto STREAM_CARD_PREFIX = "streamCard_";
    
    // Timeouts (ms)
    constexpr int TIMEOUT_SHORT = 500;
    constexpr int TIMEOUT_MEDIUM = 1500;
    constexpr int TIMEOUT_LONG = 5000;
    constexpr int TIMEOUT_PLAYBACK = 10000;
    
    // Ports par defaut
    constexpr quint16 DEFAULT_TWITCH_PORT = 0;  // Auto
    constexpr quint16 DEFAULT_HLS_PORT = 0;     // Auto
}
```

#### 4.2 E2EConstants.qml
Miroir QML des constantes pour les tests :
```qml
pragma Singleton
QtObject {
    readonly property string mainWindow: "mainWindow"
    readonly property int timeoutShort: 500
    // ...
}
```

### Phase 5 : Simplification des scenarios

#### 5.1 Template QML standard
Creer `E2EScenarioTemplate.qml` qui encapsule le boilerplate :
- Loader pour main.qml
- Initialisation du mainWindow
- Wait de stabilisation initial

#### 5.2 Helpers additionnels dans E2ETestCase.qml
- `waitForAppReady()` : attend que l'app soit completement initialisee
- `navigateTo(view)` : navigue vers une vue et attend la stabilisation
- `clickAndWait(element, timeout)` : clique et attend un changement
- `verifyStreamCard(index)` : verifie qu'une carte stream existe et est valide

## Fichiers concernes

### Nouveaux fichiers
- `tests/e2e/contexts/BaseE2EContext.hpp`
- `tests/e2e/contexts/BaseE2EContext.cpp`
- `tests/e2e/servers/MockHttpServer.hpp`
- `tests/e2e/servers/MockHttpServer.cpp`
- `tests/e2e/fixtures/FixtureLoader.hpp`
- `tests/e2e/fixtures/FixtureLoader.cpp`
- `tests/e2e/E2EConstants.hpp`
- `tests/e2e/helpers/E2EConstants.qml`
- `tests/e2e/helpers/E2EScenarioTemplate.qml`

### Fichiers a modifier
- `tests/e2e/contexts/authenticated/AuthenticatedSetup.cpp` (simplifier)
- `tests/e2e/contexts/unauthenticated/UnauthenticatedSetup.cpp` (simplifier)
- `tests/e2e/servers/MockTwitchServer.cpp` (simplifier)
- `tests/e2e/servers/MockHlsServer.cpp` (simplifier)
- `tests/e2e/helpers/E2ETestCase.qml` (enrichir)
- `tests/e2e/scenarios/open-stream/tst_open_stream.qml` (utiliser template)
- `tests/e2e/scenarios/login-view/tst_login_view.qml` (utiliser template)
- `tests/e2e/CMakeLists.txt` (ajouter E2E_FIXTURES_PATH)

## Sous-taches

- **43.1** - BaseE2EContext : Classe de base pour les contexts
- **43.2** - MockHttpServer : Classe de base pour les mock servers
- **43.3** - FixtureLoader : Chargeur centralise de fixtures
- **43.4** - E2EConstants : Centralisation des constantes C++ et QML
- **43.5** - E2ETestCase enrichi : Helpers additionnels
- **43.6** - Migration scenarios : Utiliser le template et les nouvelles classes

## Priorite des sous-taches

| Priorite | Sous-tache | Dependances | Impact |
|----------|------------|-------------|--------|
| 1 | 43.3 - FixtureLoader | Aucune | Prerequis pour 43.1 |
| 2 | 43.4 - E2EConstants | Aucune | Utilise partout |
| 3 | 43.2 - MockHttpServer | Aucune | Independant |
| 4 | 43.1 - BaseE2EContext | 43.3, 43.4 | Plus gros gain |
| 5 | 43.5 - E2ETestCase enrichi | 43.4 | Ameliore l'ergonomie |
| 6 | 43.6 - Migration scenarios | 43.1, 43.5 | Finalisation |

## Checklist de validation

### Phase 1 : Contexts
- [x] BaseE2EContext compile et est testable
- [x] AuthenticatedSetup herite de BaseE2EContext
- [x] UnauthenticatedSetup herite de BaseE2EContext
- [x] Tous les tests E2E existants passent (2/2 scenarios)
- [x] Reduction du code context >= 25% → **96% obtenu**

### Phase 2 : Mock Servers
- [x] MockHttpServer compile et est testable
- [x] MockTwitchServer herite de MockHttpServer
- [x] MockHlsServer herite de MockHttpServer
- [x] Tous les endpoints API repondent correctement
- [x] Reduction du code servers >= 10% → **38% obtenu**

### Phase 3 : Fixtures
- [x] FixtureLoader charge auth_token.json
- [x] FixtureLoader charge streams.json
- [x] FixtureLoader charge users.json
- [x] FixtureLoader charge test_segment.ts
- [x] Erreur claire si fixture manquante
- [x] E2E_FIXTURES_PATH configurable via CMake

### Phase 4 : Constantes
- [x] E2EConstants.hpp contient toutes les constantes C++
- [x] E2EConstants.qml accessible depuis les tests QML
- [x] Plus de magic strings dans le code

### Phase 5 : Scenarios
- [x] E2ETestCase a les nouveaux helpers
- [x] E2EScenarioTemplate utilise dans les scenarios
- [x] Code scenario reduit de >= 20%

### Validation finale
- [x] `ctest -L e2e` execute tous les tests avec succes (2/2)
- [x] Pas de regression (35/35 tests passent)
- [x] Documentation mise a jour (docstrings)

## Metriques de succes

| Metrique | Avant | Objectif |
|----------|-------|----------|
| Lignes totales infrastructure | ~2000 | < 1600 (-20%) |
| Lignes par nouveau scenario | ~150 | < 80 (-45%) |
| Fichiers de duplication | 4 | 0 |
| Magic strings | ~15 | 0 |
| Temps ajout nouveau context | ~30 min | < 10 min |
