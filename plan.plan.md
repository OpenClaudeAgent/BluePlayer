<!-- 11b44143-102d-4652-b1db-058744e87305 991da610-6ef6-4b46-9731-0d5634d21142 -->
# Plan : Système de couverture de tests et mutation testing

## Phase 1 : Préparation Git

1. **Merger la branche actuelle vers main**

- Vérifier que tous les tests passent sur `refactor/modularisation-network-layer`
- Merger vers `main`
- Créer une nouvelle branche `feature/test-coverage-system`

## Phase 2 : Configuration de la couverture de code avec llvm-cov

2. **Ajouter le support llvm-cov dans CMake**

- Modifier `CMakeLists.txt` pour détecter Clang et activer les flags de couverture (`-fprofile-instr-generate`, `-fcoverage-mapping`)
- Ajouter une option `BLUEPLAYER_ENABLE_COVERAGE` pour activer/désactiver la couverture
- Configurer les flags uniquement pour les builds de test quand la couverture est activée
- S'assurer que les flags de couverture sont appliqués aux bibliothèques `blueplayer_core` et `blueplayer_media`
- Configurer la génération de `compile_commands.json` pour Mull (déjà activé avec `CMAKE_EXPORT_COMPILE_COMMANDS`)

3. **Créer des scripts de génération de rapports**

- Script `scripts/generate_coverage.sh` pour :
  - Compiler avec les flags de couverture (`BLUEPLAYER_ENABLE_COVERAGE=ON`)
  - Exécuter les tests avec génération de profils (`*.profraw`)
  - Merger les profils avec `llvm-profdata merge`
  - Générer les rapports HTML avec `llvm-cov show` et `llvm-cov report`
  - Exclure les fichiers générés et les tests du rapport
- Ajouter une cible Makefile `make coverage` pour faciliter l'utilisation
- Créer un répertoire `coverage/` pour stocker les rapports

4. **Configurer l'exclusion de fichiers**

- Exclure les fichiers générés (moc_, autogen, etc.)
- Exclure les fichiers de test eux-mêmes du rapport
- Exclure les fichiers système/Qt si nécessaire

## Phase 3 : Configuration du mutation testing avec Mull

5. **Installer et configurer Mull**

- Installer Mull (binaires précompilés depuis GitHub Releases ou compilation depuis sources)
- Dépendances requises :
  - LLVM 19 : `brew install llvm@19`
  - Les binaires Mull nécessitent LLVM 19 spécifiquement
  - `llvm-cov` et `llvm-profdata` disponibles via Xcode (ou LLVM complet)
- Créer un script `scripts/run_mutation_tests.sh` pour exécuter Mull
- Configurer Mull pour cibler les bibliothèques `blueplayer_core` et `blueplayer_media`
- Configurer Mull pour ignorer les fichiers générés par Qt (moc_, qrc_, etc.)
- Créer un fichier de configuration Mull (`.mull.yml` ou `mull.yml`) pour définir les mutateurs et filtres

6. **Intégrer Mull dans CMake**

- Ajouter une option `BLUEPLAYER_ENABLE_MUTATION_TESTING`
- Créer une cible CMake pour exécuter Mull après les tests unitaires
- S'assurer que les bibliothèques sont compilées avec les flags nécessaires pour Mull (déjà géré par `CMAKE_EXPORT_COMPILE_COMMANDS`)
- Configurer Mull pour utiliser `compile_commands.json` généré par CMake

## Phase 4 : Amélioration des tests existants

7. **Analyser la couverture actuelle**

- Générer un premier rapport de couverture pour identifier les zones non testées
- Documenter les modules avec faible couverture

8. **Améliorer les tests existants**

- `tests/api/twitch/TestTwitchApiClient.cpp` : Ajouter des cas limites, gestion d'erreurs
- `tests/api/twitch/TestTwitchAuthManager.cpp` : Tests d'expiration, refresh tokens
- `tests/media/TestFFmpegMediaService.cpp` : Tests de différents formats, gestion d'erreurs FFmpeg

9. **Ajouter des tests manquants pour les modules core**

- `core/Config.cpp` : Tests de chargement/sauvegarde de configuration
- `core/Error.cpp` et `core/ErrorHandler.cpp` : Tests de gestion d'erreurs
- `core/InputValidator.cpp` : Tests de validation d'entrées
- `core/Logger.cpp` : Tests de logging (si testable)
- `core/NetworkCache.cpp` : Tests de cache réseau
- `core/SecureStorage.cpp` : Tests de stockage sécurisé
- `core/network/HttpClient.cpp` : Tests de requêtes HTTP
- `core/network/ApiClientBase.cpp` : Tests de la classe de base

10. **Ajouter des tests pour les modules media**

- `media/FFmpegBridge.cpp` : Tests d'initialisation et de gestion FFmpeg
- `media/FFmpegMediaSource.cpp` : Tests de sources média

11. **Ajouter des tests pour les ViewModels**

- `ui/HomeViewModel.cpp` : Tests de logique métier du ViewModel

## Phase 5 : Automatisation et CI/CD

12. **Ajouter des cibles Makefile**

- `make coverage` : Génère le rapport de couverture HTML (utilise `scripts/generate_coverage.sh`)
- `make mutation-test` : Exécute les tests de mutation (utilise `scripts/run_mutation_tests.sh`)
- `make test-all` : Exécute tests + couverture + mutation (optionnel)
- `make clean-coverage` : Nettoie les fichiers de couverture
- `make clean-mutation` : Nettoie les rapports de mutation

13. **Configurer des builds de validation**

- Créer un script `scripts/validate_build.sh` qui :
- Compile le projet
- Exécute tous les tests
- Génère le rapport de couverture
- Vérifie un seuil minimum de couverture (ex: 70%)
- Ajouter une cible `make validate` dans le Makefile

14. **Documentation**

- Mettre à jour `docs/SETUP.md` avec les instructions pour :
- Installer les dépendances (Mull, LLVM 19)
- Générer les rapports de couverture
- Exécuter les tests de mutation
- Créer `docs/TESTING.md` avec la stratégie de test et les conventions

## Phase 6 : Intégration continue (optionnel)

15. **Ajouter au .gitignore**

- Dossiers de rapports : `coverage/`, `mutation-reports/`
- Fichiers de profilage : `*.profdata`, `*.profraw`
- Répertoire temporaire : `build/mull-download/`
- Répertoire `usr/` si créé par erreur
- Fichiers de compilation Mull : `*.bc` (bitcode), `*.ll` (LLVM IR)

16. **Créer un workflow GitHub Actions (si applicable)**

- Build avec couverture activée
- Génération et upload des rapports
- Vérification des seuils de couverture

## Fichiers à modifier/créer

**Modifications :**

- `CMakeLists.txt` : Ajout des options et flags de couverture
- `tests/CMakeLists.txt` : Configuration pour Mull
- `Makefile` : Nouvelles cibles coverage, mutation-test, validate
- `.gitignore` : Exclusion des fichiers de rapports

**Nouveaux fichiers :**

**Scripts :**
- `scripts/generate_coverage.sh` : Script de génération de couverture
- `scripts/run_mutation_tests.sh` : Script pour exécuter Mull
- `scripts/validate_build.sh` : Script de validation complète

**Configuration :**
- `mull.yml` ou `.mull.yml` : Configuration Mull (mutateurs, filtres, exclusions)

**Tests unitaires :**
- `tests/core/TestConfig.cpp` : Tests pour Config
- `tests/core/TestErrorHandler.cpp` : Tests pour ErrorHandler
- `tests/core/TestInputValidator.cpp` : Tests pour InputValidator
- `tests/core/TestNetworkCache.cpp` : Tests pour NetworkCache
- `tests/core/TestSecureStorage.cpp` : Tests pour SecureStorage
- `tests/core/network/TestHttpClient.cpp` : Tests pour HttpClient
- `tests/core/network/TestApiClientBase.cpp` : Tests pour ApiClientBase
- `tests/media/TestFFmpegBridge.cpp` : Tests pour FFmpegBridge
- `tests/media/TestFFmpegMediaSource.cpp` : Tests pour FFmpegMediaSource
- `tests/ui/TestHomeViewModel.cpp` : Tests pour HomeViewModel

**Documentation :**
- `docs/TESTING.md` : Documentation sur la stratégie de test et les conventions

## Dépendances requises

**Pour Mull :**
- LLVM 19 : `brew install llvm@19`
- Mull binaires : Installer depuis GitHub Releases ou compiler depuis sources
  - `mull-runner-19` : Exécutable principal
  - `mull-reporter-19` : Génération de rapports
  - `mull-ir-frontend-19` : Bibliothèque frontend

**Pour la couverture de code :**
- Clang avec support de couverture
- `llvm-cov` : Disponible via Xcode (`/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/llvm-cov`)
- `llvm-profdata` : Disponible via Xcode

## Métriques cibles

- Couverture de code initiale : Identifier les zones à améliorer
- Objectif de couverture : 70%+ pour les modules core et media
- Mutation testing : Identifier les tests faibles ou manquants

## Considérations spécifiques à Qt/QML

**Pour la couverture de code :**
- Les fichiers générés par Qt (moc_, qrc_, uic_) doivent être exclus des rapports
- Les fichiers QML (.qml) ne sont pas couverts par llvm-cov (C/C++ uniquement)
- Se concentrer sur la couverture du code C++ (ViewModels, services, core)

**Pour Mull :**
- Mull fonctionne uniquement sur le code C++ compilé
- Les fichiers générés par Qt doivent être exclus de l'analyse de mutation
- Configurer Mull pour ignorer les fichiers dans `build/src/*_autogen/`
- Les tests QML ne peuvent pas être utilisés directement avec Mull (utiliser les tests unitaires C++)

**Ordre d'exécution recommandé :**
1. Générer la couverture de code pour identifier les zones non testées
2. Améliorer les tests existants et ajouter les tests manquants
3. Exécuter les tests de mutation pour identifier les tests faibles
4. Améliorer les tests basés sur les résultats de mutation
5. Répéter jusqu'à atteindre les objectifs de couverture et de mutation score

