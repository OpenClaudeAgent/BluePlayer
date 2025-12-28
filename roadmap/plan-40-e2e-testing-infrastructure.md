# Plan 40 - Infrastructure de Tests End-to-End (E2E)

## Contexte

BluePlayer dispose actuellement d'une bonne couverture de tests unitaires (~60%) et de tests fonctionnels QML pour les composants isolés, mais aucune infrastructure pour tester l'application de bout en bout. Les tests E2E sont essentiels pour valider les flux utilisateur complets comme "ouvrir un live et vérifier qu'il se charge".

L'infrastructure E2E doit permettre de :
- Spawner l'application avec des services tiers mockés (Twitch API, HLS streams)
- Interagir avec l'UI de manière automatisée
- Vérifier que les flux critiques fonctionnent correctement

## Objectif

Mettre en place une infrastructure de tests E2E permettant de :
1. Lancer l'application en mode test avec injection de mocks
2. Simuler l'API Twitch (authentification, streams, channels)
3. Simuler des streams HLS pour la lecture vidéo
4. Valider le premier use case : **ouvrir un live et vérifier son chargement**

## Specifications

### Architecture E2E proposee

```
                    +-------------------+
                    |   Test Runner     |
                    | (Qt Quick Test)   |
                    +--------+----------+
                             |
              +--------------+--------------+
              |                             |
    +---------v---------+         +---------v---------+
    |   BluePlayer App  |         |   Mock Servers    |
    | (Mode Test)       |         |                   |
    +-------------------+         +-------------------+
              |                             |
              |    HTTP Requests            |
              +------------+----------------+
                           |
              +------------v------------+
              |    MockTwitchServer     |
              | - /oauth2/token         |
              | - /helix/streams        |
              | - /helix/users          |
              | - /api/playlist.m3u8    |
              +-------------------------+
```

### Composants a implementer

#### 1. MockTwitchServer (C++)
Serveur HTTP local simulant l'API Twitch :
- Endpoints OAuth (`/oauth2/token`, `/oauth2/validate`)
- Endpoints Helix (`/helix/streams`, `/helix/users`, `/helix/channels`)
- Endpoint HLS (`/api/channel/hls/{channel}.m3u8`)
- Reponses configurables via fixtures JSON

#### 2. MockHlsServer (C++)
Serveur HTTP local pour les flux HLS :
- Playlist maitre (`.m3u8`)
- Playlists de qualite (720p, 480p, etc.)
- Segments video factices (`.ts`)
- Simulation de latence configurable

#### 3. TestApplication (C++)
Wrapper pour lancer BluePlayer en mode test :
- Injection des URLs de mock servers
- Configuration de l'environnement de test
- Hooks pour l'initialisation et le nettoyage

#### 4. E2E Test Helpers (QML)
Utilitaires pour les tests E2E :
- Navigation entre vues
- Attente de conditions (stream charge, UI prete)
- Capture d'ecran en cas d'echec

### Premier test : Ouvrir un live

**Scenario :**
1. L'application demarre avec un utilisateur authentifie (token mocke)
2. La Home affiche une liste de streams (donnees mockees)
3. L'utilisateur clique sur le premier stream
4. Le PlayerView s'affiche
5. Le stream commence a se charger
6. **Verification** : Le stream est en cours de lecture (pas de loading infini, pas d'erreur)

### Structure des fichiers

```
tests/
  e2e/
    servers/
      MockTwitchServer.hpp
      MockTwitchServer.cpp
      MockHlsServer.hpp
      MockHlsServer.cpp
    fixtures/
      streams.json
      users.json
      channels.json
      auth_token.json
    helpers/
      E2ETestBase.hpp
      E2ETestBase.cpp
      TestApplication.hpp
      TestApplication.cpp
    scenarios/
      tst_OpenLiveStream.qml
      main.cpp
    CMakeLists.txt
```

### Configuration CMake

```cmake
# tests/e2e/CMakeLists.txt
add_executable(test_e2e_scenarios
  servers/MockTwitchServer.cpp
  servers/MockHlsServer.cpp
  helpers/E2ETestBase.cpp
  helpers/TestApplication.cpp
  scenarios/main.cpp
)

target_link_libraries(test_e2e_scenarios PRIVATE
  Qt6::Core
  Qt6::Quick
  Qt6::QuickTest
  Qt6::Test
  Qt6::Network
  Qt6::HttpServer  # Pour les mock servers
  blueplayer_core
  blueplayer_media
)
```

### Variables d'environnement pour le mode test

| Variable | Description |
|----------|-------------|
| `BLUEPLAYER_TEST_MODE` | Active le mode test |
| `BLUEPLAYER_MOCK_API_URL` | URL du MockTwitchServer |
| `BLUEPLAYER_MOCK_AUTH_TOKEN` | Token OAuth pre-configure |
| `BLUEPLAYER_SKIP_AUTH` | Skip l'ecran de login |

## Fichiers concernes

### Nouveaux fichiers
- `tests/e2e/servers/MockTwitchServer.hpp/.cpp`
- `tests/e2e/servers/MockHlsServer.hpp/.cpp`
- `tests/e2e/helpers/E2ETestBase.hpp/.cpp`
- `tests/e2e/helpers/TestApplication.hpp/.cpp`
- `tests/e2e/scenarios/tst_OpenLiveStream.qml`
- `tests/e2e/scenarios/main.cpp`
- `tests/e2e/fixtures/*.json`
- `tests/e2e/CMakeLists.txt`

### Fichiers a modifier
- `tests/CMakeLists.txt` - Ajouter le sous-repertoire e2e
- `src/core/Config.cpp` - Supporter les URLs de mock via env vars
- `src/api/twitch/TwitchApiClient.cpp` - Supporter les base URLs configurables

## Checklist de validation

### Infrastructure (Phase 1 - Terminee)
- [x] MockTwitchServer demarre et repond aux requetes
- [x] MockHlsServer sert des playlists HLS valides
- [x] Les variables d'environnement sont lues correctement
- [x] Fixtures JSON chargees (3 streams, 4 users, token)
- [x] 6 tests d'infrastructure passants

### Integration API (Phase 2 - Terminee)
- [x] Config lit BLUEPLAYER_TEST_MODE et BLUEPLAYER_MOCK_API_URL
- [x] TwitchApiClient utilise URLs configurables (buildHelixUrl)
- [x] Securite: seules les URLs localhost acceptees en mode test
- [x] Test integration: TwitchApiClient -> MockTwitchServer -> Fixtures
- [x] 7 tests E2E passants

### Tests UI complets (Phase 3 - A venir)
- [ ] L'application demarre sans erreur en mode test
- [ ] La Home affiche les streams mockes
- [ ] Le clic sur un stream ouvre PlayerView
- [ ] Le stream se charge (verification du state "playing")
- [ ] Pas de crash ou erreur pendant le flux

### CI/CD (Phase 3 - A venir)
- [ ] Les tests E2E s'executent dans le pipeline
- [x] Timeout configure (max 60s par test)
- [ ] Capture d'ecran sur echec

## Notes techniques

### Qt HTTP Server
Qt 6.4+ inclut `Qt::HttpServer` qui permet de creer des serveurs HTTP facilement :

```cpp
QHttpServer server;
server.route("/helix/streams", [](const QHttpServerRequest &request) {
    return QHttpServerResponse::fromJson(streamsFixture);
});
server.listen(QHostAddress::LocalHost, 8080);
```

### Simulation HLS
Un stream HLS minimal necessite :
1. Une playlist maitre (`.m3u8`) listant les qualites
2. Des playlists de media (`.m3u8`) listant les segments
3. Des segments video (`.ts`) - peuvent etre des fichiers vides ou mini

Pour le test, on peut utiliser un stream pre-enregistre tres court (quelques secondes).

### Mode Test dans l'application
L'application doit detecter `BLUEPLAYER_TEST_MODE=1` et :
- Utiliser les URLs de mock servers au lieu des vrais endpoints Twitch
- Desactiver les timeouts reels (accelerer les tests)
- Activer des logs detailles pour le debug

## Dependances

| Dependance | Raison |
|------------|--------|
| Qt 6.5+ | Qt::HttpServer pour les mock servers |
| Qt Quick Test | Framework de test E2E |
| FFmpeg | Decodage des segments HLS (meme mockes) |

## Risques et mitigations

| Risque | Mitigation |
|--------|------------|
| Tests lents | Timeout strict, mock servers locaux |
| Tests flaky | Attentes explicites, pas de sleep() |
| Complexite de setup | Scripts d'initialisation automatiques |
| MPV ne demarre pas sans video | Segments HLS valides (meme courts) |
