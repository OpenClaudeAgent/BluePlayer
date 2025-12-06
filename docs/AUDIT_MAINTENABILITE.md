# Audit Maintenabilité BluePlayer

Date : décembre 2024

---

## 1. Architecture et dépendances CMake

### Points forts
- Architecture modulaire claire avec deux bibliothèques principales : `blueplayer_core` et `blueplayer_media`
- Options de build configurables (tests, coverage, mutation testing, static analysis)
- Utilisation de CMake 3.24+ avec presets modernes et Ninja
- Integration de Qt 6.5+ et C++20

### Problèmes identifiés

| Priorité | Problème | Impact |
|----------|----------|--------|
| Haute | Chemins hardcodés `/opt/homebrew/*` dans CMakeLists.txt et src/CMakeLists.txt | Non portable (macOS Homebrew uniquement) |
| Haute | Dépendance circulaire `core ↔ media` résolue par forward declarations + private linking | Complexité de compilation, linker warnings masqués |
| Moyenne | `api/twitch` et `ui/HomeViewModel` inclus dans `blueplayer_core` au lieu de modules séparés | Couplage excessif, temps de compilation |
| Moyenne | Makefile avec `CMAKE_EXECUTABLE` hardcodé (`/opt/homebrew/bin/cmake`) | Non portable |
| Basse | Pas de CMakeLists.txt séparé pour `src/api/twitch` | Moins lisible pour les contributeurs |

### Recommandations prioritaires

1. **Remplacer les chemins hardcodés** par des variables `find_package` ou des cibles importées :
```cmake
# Au lieu de
target_include_directories(blueplayer_media PUBLIC /opt/homebrew/include)
target_link_directories(blueplayer_media PRIVATE /opt/homebrew/lib)

# Préférer
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
target_link_libraries(blueplayer_media PUBLIC PkgConfig::MPV)
```

2. **Découpler les modules** : déplacer `api/twitch` vers une bibliothèque `blueplayer_api` et `ui/HomeViewModel` vers `blueplayer_ui`.

3. **Utiliser `cmake` du PATH** dans le Makefile au lieu d'un chemin absolu.

---

## 2. Qualité du code C++/QML

### Points forts
- Utilisation cohérente des namespaces (`blueplayer::core`, `blueplayer::api::twitch`)
- Forward declarations pour éviter les dépendances circulaires
- `std::unique_ptr` pour la gestion mémoire (RAII)
- Attribut `[[nodiscard]]` utilisé
- `.clang-format` complet avec style cohérent (LLVM + modifications)
- `.clang-tidy` avec checks pertinents

### Problèmes identifiés

| Priorité | Problème | Fichier(s) |
|----------|----------|------------|
| Haute | `WarningsAsErrors: ''` dans .clang-tidy (désactivé) | .clang-tidy |
| Haute | Beaucoup de logs debug avec préfixe `[DEBUG]` hardcodé dans le message | TwitchService.cpp, HomeView.qml |
| Moyenne | Certains getters non marqués `const` | Divers |
| Moyenne | Absence de documentation Doxygen sur les méthodes publiques de TwitchService | TwitchService.hpp |
| Basse | Utilisation de `qgetenv` directement dans TwitchService au lieu de Config | TwitchService.cpp:27 |
| Basse | Magic numbers (ex: `20` pour le nombre de streams) | TwitchService.cpp |

### Recommandations

1. **Activer `WarningsAsErrors`** dans `.clang-tidy` pour au moins les checks critiques :
```yaml
WarningsAsErrors: 'bugprone-*,cert-*'
```

2. **Uniformiser les logs** : supprimer les préfixes `[DEBUG]` hardcodés et laisser le Logger gérer le niveau :
```cpp
// Au lieu de
Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] message"));

// Préférer
Logger::debug(LogCategory::Twitch, QStringLiteral("message"));
```

3. **Extraire les constantes** vers `Constants.hpp` :
```cpp
namespace constants::api {
  inline constexpr int kDefaultStreamCount = 20;
  inline constexpr int kDefaultClipCount = 5;
}
```

4. **Utiliser Config::instance()** dans TwitchService au lieu de `qgetenv`.

---

## 3. Gestion des erreurs et logs

### Points forts
- Système `Error` / `ErrorHandler` bien structuré avec codes typés
- Messages localisés en français
- Conversion implicite vers `QString` pour compatibilité QML
- Logger avec catégories (`Media`, `Twitch`, `UI`, `Core`, `Network`)
- Configuration des niveaux via variables d'environnement

### Problèmes identifiés

| Priorité | Problème | Impact |
|----------|----------|--------|
| Haute | `Error` n'est pas un QObject : impossible de propager nativement vers QML | Perte de contexte côté UI |
| Moyenne | Signaux `errorOccurred(QString)` perdent le code d'erreur | Pas de distinction par type côté UI |
| Moyenne | Pas de mécanisme de retry/backoff dans TwitchService | Erreurs réseau non récupérables |
| Basse | Logs debug très verbeux (ralentit la lecture en production) | Performance logs |

### Recommandations

1. **Enrichir le signal d'erreur QML** avec le code :
```cpp
Q_INVOKABLE void emitError(const Error& error);
// Signal: errorOccurred(int code, QString message)
```

2. **Ajouter un niveau de log `Trace`** pour les messages très verbeux et les désactiver par défaut.

3. **Implémenter un retry avec backoff exponentiel** dans `ApiClientBase` pour les erreurs 5xx et timeouts.

---

## 4. Configuration et secrets

### Points forts
- Singleton `Config` chargeant depuis env + fichier JSON
- `SecureStorage` avec chiffrement XOR dérivé de l'UUID système (+ Keychain sur macOS)
- Validation des ports et conversions robustes
- Script `load_env.sh` pour charger `.env`

### Problèmes identifiés

| Priorité | Problème | Impact |
|----------|----------|--------|
| Haute | Chiffrement XOR dans SecureStorage (faible sécurité) | Tokens accessibles avec clé système |
| Haute | `clientSecret` peut être stocké dans config.json en clair | Fuite potentielle |
| Moyenne | Pas de validation du contenu du fichier config.json | Crash si JSON malformé |
| Basse | Salt hardcodé dans SecureStorage (`BluePlayerSecureStorage2024`) | Réduire en cas d'audit externe |

### Recommandations

1. **Remplacer XOR par AES-256-GCM** via `QCA` (Qt Cryptographic Architecture) ou `libsodium` :
```cpp
// Exemple avec libsodium (pseudo-code)
crypto_secretbox_easy(ciphertext, plaintext, len, nonce, key);
```

2. **Interdire `clientSecret` dans config.json** : ne l'accepter que depuis les variables d'environnement.

3. **Valider le JSON** avant de l'utiliser :
```cpp
QJsonParseError error;
const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
if (error.error != QJsonParseError::NoError) {
  Logger::error(LogCategory::Core, QStringLiteral("Config JSON invalide: %1").arg(error.errorString()));
  return;
}
```

---

## 5. Tests et couverture

### Points forts
- Suite de tests Qt Test Framework avec helpers (`TestHelpers`)
- Tests unitaires pour : ErrorHandler, InputValidator, NetworkCache, SecureStorage, HttpClient, ApiClientBase, Config
- Tests API : TwitchApiClient, TwitchAuthManager
- Tests intégration : TwitchFlow
- Tests UI : HomeViewModel
- Script `validate_build.sh` avec seuil de couverture (70%)

### Gaps identifiés

| Module | Couverture | Priorité |
|--------|------------|----------|
| `TwitchService` | Non testé directement (seulement via intégration) | Haute |
| `HlsAdFilter` | Non testé | Haute |
| `MpvQuickItem` | Non testé (difficile sans GPU) | Moyenne |
| `Application` | Non testé | Moyenne |
| `WatchHistory` | Non testé | Basse |
| QML (`HomeView.qml`, `PlayerView.qml`) | Non testé | Basse (manuel ou qmltest) |

### Recommandations

1. **Ajouter des tests unitaires pour TwitchService** en mockant `TwitchApiClient` et `TwitchAuthManager` :
```cpp
class MockTwitchApiClient : public TwitchApiClient {
  // Override signals/slots pour simuler les réponses
};
```

2. **Tester HlsAdFilter** avec des fixtures de playlists HLS.

3. **Augmenter le seuil de couverture** à 80% après avoir comblé les gaps.

4. **Ajouter des tests QML** avec `qmltest` ou `Squish` pour les interactions UI.

---

## 6. Build, formatage et CI locale

### Points forts
- Scripts bien structurés (`format.sh`, `analyze.sh`, `generate_coverage.sh`, `validate_build.sh`)
- Utilisation de `set -e` pour arrêter en cas d'erreur
- Rapport de couverture HTML avec llvm-cov
- Cibles Makefile claires

### Problèmes identifiés

| Priorité | Problème | Impact |
|----------|----------|--------|
| Haute | Pas de cible `make format` ni `make lint` | Pas de vérification automatique |
| Haute | Pas de hook pre-commit | Code non formaté peut être commité |
| Moyenne | `format.sh` ne formate pas les fichiers tests | Incohérence de style |
| Basse | `analyze.sh` tronque la sortie (`head -20`, `head -50`) | Peut masquer des erreurs |

### Recommandations

1. **Ajouter des cibles Makefile** :
```makefile
.PHONY: format
format:
	@$(SCRIPTS_DIR)/format.sh

.PHONY: lint
lint:
	@$(SCRIPTS_DIR)/analyze.sh

.PHONY: format-check
format-check:
	@clang-format --dry-run --Werror $(shell find src tests -name "*.cpp" -o -name "*.hpp")
```

2. **Installer un hook pre-commit** (via le package `pre-commit` ou un script `.git/hooks/pre-commit`) :
```bash
#!/bin/bash
make format-check || { echo "Code non formaté. Exécutez 'make format'."; exit 1; }
```

3. **Formater aussi les tests** dans `format.sh` :
```bash
find "$PROJECT_ROOT/src" "$PROJECT_ROOT/tests" -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +
```

4. **Supprimer les troncatures** dans `analyze.sh` pour voir toutes les erreurs.

5. **Ajouter `-Werror` en option** dans CMake pour les builds CI :
```cmake
option(BLUEPLAYER_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
if(BLUEPLAYER_WARNINGS_AS_ERRORS)
  target_compile_options(blueplayer_core PRIVATE -Werror)
endif()
```

---

## Résumé des actions prioritaires

| # | Action | Effort | Impact |
|---|--------|--------|--------|
| 1 | Remplacer chemins hardcodés par find_package | Moyen | Portabilité |
| 2 | Activer WarningsAsErrors (clang-tidy) | Faible | Qualité code |
| 3 | Ajouter cibles format/lint au Makefile | Faible | DX |
| 4 | Hook pre-commit pour format-check | Faible | Qualité commits |
| 5 | Tests unitaires TwitchService + HlsAdFilter | Moyen | Fiabilité |
| 6 | Remplacer XOR par AES dans SecureStorage | Moyen | Sécurité |
| 7 | Découpler modules (api, ui) | Élevé | Maintenabilité |
| 8 | Uniformiser les logs (supprimer [DEBUG]) | Faible | Lisibilité |

---

## Annexe : Versions des outils

| Outil | Version recommandée |
|-------|---------------------|
| CMake | >= 3.24 |
| Clang | >= 15 |
| Qt | 6.5+ |
| clang-format | >= 15 |
| clang-tidy | >= 15 |
| llvm-cov | >= 15 |
