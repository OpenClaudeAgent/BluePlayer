# Guide d'Installation et Configuration

Ce guide explique comment configurer et exécuter les tests fonctionnels UI de BluePlayer.

## Prérequis

### Dépendances Qt

Les tests fonctionnels nécessitent le module **Qt Quick Test** :

```bash
# macOS avec Homebrew
brew install qt@6

# Vérifier que QuickTest est disponible
ls $(brew --prefix qt@6)/lib/cmake/Qt6QuickTest
```

### Version minimale

- **Qt** : 6.5+
- **CMake** : 3.24+
- **Compilateur** : Clang 14+ ou GCC 11+

## Configuration CMake

### Ajout au projet

Les tests fonctionnels sont configurés dans `tests/CMakeLists.txt` :

```cmake
# ===== Tests Fonctionnels UI (Qt Quick Test) =====

find_package(Qt6 6.5 COMPONENTS QuickTest REQUIRED)

add_executable(test_functional_ui
  functional/main.cpp
)

target_link_libraries(test_functional_ui PRIVATE
  Qt6::Core
  Qt6::Quick
  Qt6::Gui
  Qt6::QuickTest
  Qt6::Test
)

target_compile_definitions(test_functional_ui PRIVATE
  QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/functional"
)

add_test(NAME FunctionalUI COMMAND test_functional_ui)
```

### Variables importantes

| Variable | Description |
|----------|-------------|
| `QUICK_TEST_SOURCE_DIR` | Répertoire contenant les fichiers `tst_*.qml` |
| `QML_IMPORT_PATH` | Chemins pour trouver les composants QML de l'application |

## Build

### Build standard

```bash
# Configuration
cmake -B build -DBLUEPLAYER_ENABLE_TESTS=ON

# Compilation
cmake --build build

# Vérifier que l'exécutable existe
ls -la build/tests/test_functional_ui
```

### Build avec couverture

```bash
cmake -B build \
  -DBLUEPLAYER_ENABLE_TESTS=ON \
  -DBLUEPLAYER_ENABLE_COVERAGE=ON

cmake --build build
```

## Exécution des tests

### Commandes de base

```bash
# Tous les tests
./build/tests/test_functional_ui

# Lister les tests disponibles
./build/tests/test_functional_ui -functions

# Un test spécifique
./build/tests/test_functional_ui PlayerControlBarTests::test_playPauseButton_click_emitsSignal

# Un TestCase complet
./build/tests/test_functional_ui PlayerControlBarTests
```

### Options utiles

| Option | Description |
|--------|-------------|
| `-v1` | Affiche les noms des tests |
| `-v2` | Affiche les noms + résultats détaillés |
| `-vs` | Affiche les signaux émis |
| `-functions` | Liste toutes les fonctions de test |
| `-datatags` | Liste les data tags pour les tests data-driven |
| `-o filename.xml,xml` | Export résultats en XML (JUnit) |
| `-o filename.txt,txt` | Export résultats en texte |

### Exemples avancés

```bash
# Mode très verbose avec signaux
./build/tests/test_functional_ui -v2 -vs

# Export pour CI (format JUnit)
./build/tests/test_functional_ui -o results.xml,junitxml

# Timeout personnalisé (ms)
./build/tests/test_functional_ui -timeout 60000

# Exécuter via CTest
cd build && ctest -R FunctionalUI -V
```

## Intégration CTest

Les tests sont automatiquement enregistrés avec CTest :

```bash
cd build

# Lister tous les tests
ctest -N

# Exécuter les tests fonctionnels
ctest -R FunctionalUI --output-on-failure

# Avec verbose
ctest -R FunctionalUI -V
```

## Debugging des tests

### Problèmes courants

#### 1. "QML module not found"

**Symptôme** : Erreur `module "..." is not installed`

**Solution** : Vérifier les chemins d'import dans `main.cpp` :

```cpp
void qmlEngineAvailable(QQmlEngine* engine) {
    engine->addImportPath("/chemin/vers/src/ui");
    qDebug() << engine->importPathList();  // Debug
}
```

#### 2. "Component is not ready"

**Symptôme** : Le composant ne se crée pas

**Solution** : Utiliser `createTemporaryObject` avec vérification :

```qml
function init() {
    controlBar = createTemporaryObject(controlBarComponent, testCase)
    verify(controlBar !== null, "Component should be created")
    verify(controlBar.status === Component.Ready, "Component should be ready")
}
```

#### 3. Tests flaky (intermittents)

**Symptôme** : Tests qui passent/échouent aléatoirement

**Solutions** :
- Ajouter `waitForRendering(item)` après les changements visuels
- Utiliser `tryCompare()` au lieu de `compare()` pour les valeurs asynchrones
- Augmenter les délais avec `wait(ms)` si nécessaire

```qml
// Mauvais - peut être flaky
mouseClick(button)
compare(spy.count, 1)

// Mieux - attend que la condition soit vraie
mouseClick(button)
tryCompare(spy, "count", 1, 1000)  // timeout 1s
```

### Logs de debug

```qml
function test_example() {
    console.log("Test starting...")
    console.log("Button position:", button.x, button.y)
    console.log("Button visible:", button.visible)
    
    mouseClick(button)
    
    console.log("Signal count:", spy.count)
    console.log("Signal args:", JSON.stringify(spy.signalArguments))
}
```

Exécuter avec `-v2` pour voir les logs :

```bash
./build/tests/test_functional_ui -v2 2>&1 | grep "qml:"
```

## Structure d'un fichier de test

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

TestCase {
    id: testCase
    name: "MyComponentTests"    // Nom affiché dans les résultats
    when: windowShown           // Attend que la fenêtre soit visible
    width: 800                  // Dimensions de la fenêtre de test
    height: 600

    // Composant à tester (inline ou importé)
    Component {
        id: componentUnderTest
        MyComponent { }
    }

    // Instance et spies
    property var instance: null
    SignalSpy { id: mySpy; signalName: "mySignal" }

    // Setup avant chaque test
    function init() {
        instance = createTemporaryObject(componentUnderTest, testCase)
        mySpy.target = instance
        mySpy.clear()
        waitForRendering(instance)
    }

    // Cleanup après chaque test (automatique avec createTemporaryObject)
    function cleanup() {
        instance = null
    }

    // Tests
    function test_something() {
        // Arrange
        // Act
        // Assert
    }
}
```

## Environnement CI

### GitHub Actions

```yaml
- name: Run Functional UI Tests
  run: |
    cd build
    # Utiliser xvfb pour les tests GUI en headless
    xvfb-run --auto-servernum ./tests/test_functional_ui -o results.xml,junitxml
    
- name: Upload Test Results
  uses: actions/upload-artifact@v3
  with:
    name: functional-test-results
    path: build/results.xml
```

### macOS (pas besoin de xvfb)

```yaml
- name: Run Functional UI Tests
  run: |
    ./build/tests/test_functional_ui -o results.xml,junitxml
```

## Ressources

- [Qt Quick Test Documentation](https://doc.qt.io/qt-6/qtquicktest-index.html)
- [TestCase QML Type](https://doc.qt.io/qt-6/qml-qttest-testcase.html)
- [SignalSpy QML Type](https://doc.qt.io/qt-6/qml-qttest-signalspy.html)
