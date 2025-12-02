# Guide de test pour BluePlayer

Ce document décrit la stratégie de test, les conventions et l'utilisation des outils de couverture et de mutation testing pour BluePlayer.

## Vue d'ensemble

BluePlayer utilise une approche multi-niveaux pour les tests :

1. **Tests unitaires** : Testent les composants individuels en isolation
2. **Tests d'intégration** : Vérifient l'interaction entre les modules
3. **Couverture de code** : Mesure le pourcentage de code exécuté par les tests
4. **Mutation testing** : Identifie les tests faibles ou manquants

## Framework de test

BluePlayer utilise **Qt Test Framework** pour tous les tests. Chaque fichier de test suit cette structure :

```cpp
#include <QtTest/QtTest>
#include "module/ClassToTest.hpp"

class TestClassName : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  
  void testMethod1();
  void testMethod2();
};

void TestClassName::initTestCase() {
  // Initialisation avant tous les tests
}

void TestClassName::cleanupTestCase() {
  // Nettoyage après tous les tests
}

void TestClassName::testMethod1() {
  // Test spécifique
  QVERIFY(condition);
  QCOMPARE(actual, expected);
}

QTEST_MAIN(TestClassName)
#include "TestClassName.moc"
```

## Conventions de nommage

- **Fichiers de test** : `Test<ModuleName>.cpp` (ex: `TestConfig.cpp`)
- **Classes de test** : `Test<ClassName>` (ex: `TestConfig`)
- **Méthodes de test** : `test<MethodName>()` (ex: `testLoadFromEnvironment()`)
- **Emplacement** : Les tests sont organisés dans `tests/` avec la même structure que `src/`

## Structure des tests

```
tests/
├── api/
│   └── twitch/
│       ├── TestTwitchApiClient.cpp
│       └── TestTwitchAuthManager.cpp
├── core/
│   ├── TestConfig.cpp
│   ├── TestErrorHandler.cpp
│   ├── TestInputValidator.cpp
│   ├── TestNetworkCache.cpp
│   ├── TestSecureStorage.cpp
│   └── network/
│       ├── TestHttpClient.cpp
│       └── TestApiClientBase.cpp
├── media/
│   ├── TestFFmpegMediaService.cpp
│   ├── TestFFmpegBridge.cpp
│   └── TestFFmpegMediaSource.cpp
├── ui/
│   └── TestHomeViewModel.cpp
└── integration/
    ├── TestTwitchFlow.cpp
    └── TestMediaPipeline.cpp
```

## Exécution des tests

### Tous les tests

```bash
make test
```

### Tests spécifiques

```bash
cd build
ctest -R TestName
```

### Avec sortie détaillée

```bash
cd build
ctest --output-on-failure
```

## Couverture de code

### Objectifs

- **Objectif minimum** : 70% de couverture pour les modules `core` et `media`
- **Objectif idéal** : 80%+ de couverture

### Génération du rapport

```bash
make coverage
```

Le rapport HTML est généré dans `coverage/html/index.html`.

### Exclusions

Les fichiers suivants sont automatiquement exclus des rapports :

- Fichiers générés par Qt : `moc_*`, `qrc_*`, `uic_*`, `*_autogen/*`
- Fichiers de test : `tests/**`, `test_*`, `Test*.cpp`
- Fichiers système : `main.mm`, `main.cpp`

### Analyse du rapport

1. Ouvrez `coverage/html/index.html` dans un navigateur
2. Naviguez vers les fichiers source pour voir les lignes couvertes/non couvertes
3. Identifiez les zones avec faible couverture
4. Ajoutez des tests pour améliorer la couverture

## Mutation testing

### Objectif

Le mutation testing identifie les tests qui ne détectent pas les bugs. Si un mutateur modifie le code et que les tests passent toujours, cela indique un test faible ou manquant.

### Exécution

```bash
make mutation-test
```

### Configuration

La configuration Mull est définie dans `mull.yml` :

- **Mutateurs** : Types de mutations appliquées (and/or replacement, math operations, etc.)
- **Exclusions** : Fichiers à ignorer (générés par Qt, tests, etc.)
- **Inclusions** : Chemins à analyser (`src/core/**`, `src/media/**`)

### Interprétation des résultats

- **Mutations tuées** : Bon signe - les tests ont détecté la mutation
- **Mutations survivantes** : Problème - les tests n'ont pas détecté la mutation
- **Mutations timeout** : La mutation a causé un timeout (peut indiquer une boucle infinie)

### Amélioration basée sur les résultats

1. Pour chaque mutation survivante, ajoutez ou améliorez les tests
2. Répétez jusqu'à ce que toutes les mutations soient tuées
3. Vérifiez régulièrement pour maintenir un bon score de mutation

## Bonnes pratiques

### Écriture de tests

1. **Un test, une responsabilité** : Chaque test doit vérifier une seule chose
2. **Nommage descriptif** : Les noms de tests doivent décrire ce qu'ils testent
3. **Arrange-Act-Assert** : Structurez vos tests en trois phases
4. **Tests indépendants** : Les tests ne doivent pas dépendre les uns des autres
5. **Nettoyage** : Utilisez `cleanupTestCase()` pour restaurer l'état initial

### Exemples

```cpp
void TestConfig::testLoadFromEnvironment() {
  // Arrange
  qputenv("TWITCH_CLIENT_ID", "test_id");
  
  // Act
  Config& config = Config::instance();
  config.load();
  
  // Assert
  QCOMPARE(config.twitchClientId(), QString("test_id"));
  
  // Cleanup
  qunsetenv("TWITCH_CLIENT_ID");
}
```

### Tests de gestion d'erreurs

Toujours tester les cas d'erreur :

```cpp
void TestInputValidator::testIsValidUrlInvalid() {
  QVERIFY(!InputValidator::isValidUrl(""));
  QVERIFY(!InputValidator::isValidUrl("not a url"));
  QVERIFY(!InputValidator::isValidUrl("ftp://example.com"));
}
```

### Tests avec mocks

Pour les tests nécessitant des dépendances externes (réseau, fichiers), utilisez des mocks :

```cpp
// Exemple avec QNetworkAccessManager mocké
class MockNetworkManager : public QNetworkAccessManager {
  // Implémentation mockée
};
```

## Intégration continue

### Validation automatique

Le script `scripts/validate_build.sh` peut être utilisé dans CI/CD pour :

1. Compiler le projet
2. Exécuter tous les tests
3. Générer le rapport de couverture
4. Vérifier le seuil de couverture (70%)
5. Retourner un code d'erreur si le seuil n'est pas atteint

### Workflow recommandé

1. Écrire le code
2. Écrire les tests
3. Vérifier la couverture : `make coverage`
4. Améliorer les tests si nécessaire
5. Exécuter les tests de mutation : `make mutation-test`
6. Améliorer les tests basés sur les résultats
7. Valider : `make validate`

## Métriques et objectifs

### Couverture de code

- **Minimum** : 70% pour `core` et `media`
- **Idéal** : 80%+
- **Mesure** : Pourcentage de lignes exécutées au moins une fois

### Mutation testing

- **Objectif** : Tuer toutes les mutations (100%)
- **Acceptable** : 90%+ de mutations tuées
- **Mesure** : Score de mutation = (mutations tuées / mutations totales) × 100

## Dépannage

### Les tests échouent après modification du code

1. Vérifiez que les tests sont à jour avec le code
2. Vérifiez les dépendances et les mocks
3. Exécutez avec `--output-on-failure` pour plus de détails

### La couverture est faible

1. Identifiez les fichiers avec faible couverture dans le rapport HTML
2. Ajoutez des tests pour les branches non couvertes
3. Testez les cas limites et les erreurs

### Mull ne trouve pas les binaires

1. Vérifiez que LLVM 19 est installé : `brew list llvm@19`
2. Vérifiez que Mull est dans le PATH : `which mull-runner-19`
3. Vérifiez que `compile_commands.json` existe dans `build/`

## Ressources

- [Qt Test Framework Documentation](https://doc.qt.io/qt-6/qttest-index.html)
- [llvm-cov Documentation](https://llvm.org/docs/CommandGuide/llvm-cov.html)
- [Mull Documentation](https://mull.readthedocs.io/)
- [Mutation Testing Explained](https://mull.readthedocs.io/en/latest/Introduction.html)


