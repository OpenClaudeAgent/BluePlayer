# End-to-End Testing

BluePlayer utilise Qt Quick Test pour les tests end-to-end (E2E). Ces tests valident les parcours utilisateur complets en simulant des interactions réelles avec l'interface.

## Architecture

```
tests/e2e/
├── contexts/                 # Setups réutilisables (librairies)
│   ├── authenticated/        # Contexte: utilisateur connecté
│   │   ├── AuthenticatedSetup.cpp
│   │   ├── AuthenticatedSetup.hpp
│   │   └── CMakeLists.txt
│   └── unauthenticated/      # Contexte: pas de credentials
│       ├── UnauthenticatedSetup.cpp
│       ├── UnauthenticatedSetup.hpp
│       └── CMakeLists.txt
├── scenarios/                # Flows de test individuels
│   ├── open-stream/          # Scénario: ouvrir un stream
│   │   ├── main.cpp          # Utilise AuthenticatedSetup
│   │   ├── tst_open_stream.qml
│   │   └── CMakeLists.txt
│   └── login-view/           # Scénario: page de login
│       ├── main.cpp          # Utilise UnauthenticatedSetup
│       ├── tst_login_view.qml
│       └── CMakeLists.txt
├── servers/                  # Mock servers HTTP
│   ├── MockTwitchServer      # Simule l'API Twitch
│   └── MockHlsServer         # Simule les streams HLS
├── mocks/                    # Mocks de services
│   └── MockSecureStorage.hpp # Stockage de credentials mocké
├── fixtures/                 # Données mockées (JSON)
│   ├── auth_token.json
│   ├── streams.json
│   └── users.json
├── launcher/                 # App interactive avec mock servers
│   └── main.cpp
└── CMakeLists.txt
```

## Concepts clés

### Contexts vs Scenarios

- **Context** : Une librairie statique qui configure l'environnement de test (mock servers, credentials, etc.)
- **Scenario** : Un exécutable qui utilise un context et contient les tests QML

Cette séparation permet de réutiliser les contexts pour plusieurs scénarios.

### Mock Servers

Les tests E2E n'appellent jamais les vrais serveurs Twitch. Des mock servers locaux interceptent les requêtes et retournent des données de fixtures :

- **MockTwitchServer** : Répond aux appels API (streams, users, auth) sur le port 8080
- **MockHlsServer** : Fournit des manifestes HLS de test sur le port 8081

### Contexts disponibles

| Context | Description | Port Twitch | Port HLS |
|---------|-------------|-------------|----------|
| `authenticated` | Utilisateur connecté avec token mocké | 8080 | 8081 |
| `unauthenticated` | Pas de credentials | 8082 | 8083 |

## Commandes

```bash
# Exécuter tous les tests E2E
make e2e

# Exécuter un scénario spécifique
make e2e SCENARIO=open-stream   # Scénario authenticated
make e2e SCENARIO=login-view    # Scénario unauthenticated

# Lancer l'app avec mock servers (debug interactif)
make e2e-launcher
```

**Scénarios disponibles :**
- `open-stream` - Test du flow d'ouverture de stream (authenticated)
- `login-view` - Test de la page de login (unauthenticated)

## Screenshots

Un screenshot est automatiquement capturé après chaque test via `grabToImage()`. Les images sont sauvegardées dans :

```
/tmp/e2e_screenshots/<test_name>_<timestamp>.png
```

Cela permet de :
- Debugger visuellement les échecs
- Documenter l'état de l'UI
- Détecter des régressions visuelles

## Ajouter un nouveau scénario

### 1. Créer le dossier du scénario

```bash
mkdir -p tests/e2e/scenarios/my-scenario
```

### 2. Créer main.cpp

```cpp
// tests/e2e/scenarios/my-scenario/main.cpp
#include <QtQuickTest/quicktest.h>
#include "contexts/authenticated/AuthenticatedSetup.hpp"

QUICK_TEST_MAIN_WITH_SETUP(E2E_MyScenario, blueplayer::test::e2e::AuthenticatedSetup)
```

### 3. Créer le test QML

```qml
// tests/e2e/scenarios/my-scenario/tst_my_scenario.qml
import QtQuick
import QtTest

TestCase {
    id: testCase
    name: "E2E_MyScenario"
    when: windowShown

    property var mainWindow: null

    function initTestCase() {
        console.log("=== E2E My Scenario Tests ===")
    }

    function init() {
        mainWindow = findChild(null, "mainWindow")
        if (!mainWindow) {
            wait(500)
            mainWindow = findChild(null, "mainWindow")
        }
    }

    function cleanup() {
        // Screenshot après chaque test
        if (mainWindow) {
            var timestamp = new Date().toISOString().replace(/[:.]/g, "-")
            var testName = testCase.name + "_" + timestamp
            mainWindow.grabToImage(function(image) {
                image.saveToFile("/tmp/e2e_screenshots/" + testName + ".png")
            })
        }
    }

    function test_my_feature() {
        console.log("Testing: My feature")
        wait(1000)
        
        var element = findChild(mainWindow, "myElement")
        verify(element !== null, "Element should exist")
        console.log("✓ Test passed")
    }

    function findChild(parent, objectName) {
        if (!parent) return null
        if (parent.objectName === objectName) return parent
        for (var i = 0; i < parent.children.length; i++) {
            var found = findChild(parent.children[i], objectName)
            if (found) return found
        }
        return null
    }
}
```

### 4. Créer CMakeLists.txt

```cmake
# tests/e2e/scenarios/my-scenario/CMakeLists.txt
set(CMAKE_AUTOMOC ON)

add_executable(e2e_my_scenario main.cpp)

target_link_libraries(e2e_my_scenario PRIVATE
    Qt6::Core Qt6::Quick Qt6::QuickTest
    e2e_context_authenticated  # Ou e2e_context_unauthenticated
)

target_include_directories(e2e_my_scenario PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_SOURCE_DIR}/src
)

# Copier le fichier QML
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_my_scenario.qml
    ${CMAKE_CURRENT_BINARY_DIR}/tst_my_scenario.qml
    COPYONLY
)

add_test(
    NAME e2e_my_scenario
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen
        $<TARGET_FILE:e2e_my_scenario>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
```

### 5. Enregistrer le scénario

Dans `tests/e2e/scenarios/CMakeLists.txt` :

```cmake
add_subdirectory(my-scenario)
```

## Ajouter un nouveau context

Si vous avez besoin d'un nouvel environnement de test (ex: utilisateur premium, mode offline) :

### 1. Créer le dossier du context

```bash
mkdir -p tests/e2e/contexts/my-context
```

### 2. Créer les fichiers Setup

```cpp
// MyContextSetup.hpp
class MyContextSetup : public QObject {
    Q_OBJECT
public slots:
    void applicationAvailable();
    void qmlEngineAvailable(QQmlEngine* engine);
    void cleanupTestCase();
};

// MyContextSetup.cpp
void MyContextSetup::applicationAvailable() {
    // Démarrer les mock servers
    // Configurer l'environnement
    // Créer les mocks de services
}
```

### 3. Créer CMakeLists.txt

```cmake
add_library(e2e_context_my_context STATIC
    MyContextSetup.cpp MyContextSetup.hpp
)
target_link_libraries(e2e_context_my_context PUBLIC
    Qt6::Core Qt6::Quick Qt6::QuickTest
    blueplayer_core blueplayer_media e2e_mock_servers
)
target_link_directories(e2e_context_my_context PUBLIC ${HOMEBREW_PREFIX}/lib)
```

## Bonnes pratiques

- **Isolation** : Chaque scénario est un exécutable indépendant
- **Réutilisation** : Utiliser les contexts existants quand possible
- **objectName** : Nommer les éléments QML pour les retrouver dans les tests
- **wait()** : Attendre après les actions asynchrones
- **Screenshots** : Toujours capturer en fin de test pour le debug

## Debugging

```bash
# Lancer avec logs détaillés
cd build/tests/e2e/scenarios/open-stream
QT_QPA_PLATFORM=offscreen ./e2e_open_stream -v2

# Voir les screenshots
open /tmp/e2e_screenshots/

# Lancer l'app interactive pour debug manuel
make e2e-launcher
```
