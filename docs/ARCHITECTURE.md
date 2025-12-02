# BluePlayer – Architecture de Référence

## Objectifs
- Lecteur Twitch natif, focalisé sur la performance et la faible latence.
- Exploiter C++/Qt pour l’UI et la gestion système, FFmpeg pour le décodage matériel.
- Fournir une base modulaire facilitant les évolutions (chat, overlay, instrumentation).

## Vue d'Ensemble
```
src/
  api/twitch        -> Authentification OAuth2 + récupération des manifestes HLS/DASH + service Qt exposé à QML
  core/             -> Configuration, télémétrie, utilitaires
    network/        -> Couche réseau abstraite (HttpClient, ApiClientBase)
    media           -> Bridges FFmpeg, gestion des codecs/accélérations
    system          -> Monitoring CPU/GPU, préférences utilisateur
  media/            -> Sessions média, pipelines de décodage (Note: Ce module gère les sessions média et les pipelines de décodage, tandis que src/core/media pourrait être utilisé pour des "bridges" plus bas niveau vers des librairies comme FFmpeg.)
  streaming/        -> Fetcher HLS/DASH, adaptation de qualité, cache segments
  player/           -> Orchestrateur Audio/Vidéo, synchronisation, rendu GPU
  ui/               -> Composants QML, commandes playback, statistiques
```

## Modules

### 1. `api/twitch`
- `TwitchAuthManager` : flux OAuth (PKCE), stockage sécurisé des tokens, serveur de callback local TLS (mkcert) pour les redirections HTTPS. Utilise `HttpClient` pour les requêtes réseau.
- `TwitchApiClient` : accès aux endpoints (streams live, VOD, manifestes). Hérite de `ApiClientBase` pour la gestion réseau centralisée.
- `docs/twitch-api.md` décrira les endpoints utilisés et les contraintes de quota.
- `TwitchService` : service Qt exposant l'état d'authentification, la liste des streams et la sélection de VOD vers l'UI QML.

### 2. `core`
- `Config` : configuration centralisée depuis variables d'environnement et fichiers.
- `Logger` : système de logging structuré avec catégories.
- `Error` / `ErrorHandler` : gestion d'erreurs standardisée avec codes d'erreur typés.
- `InputValidator` : validation d'entrées utilisateur (URLs, chemins, tokens).
- `SecureStorage` : stockage sécurisé des données sensibles.
- `NetworkCache` : cache réseau avec TTL configurable.
- `Application` : point d'entrée principal, orchestre les services.

#### 2.1 `core/network` (Nouveau)
- `HttpClient` : client HTTP centralisé avec gestion de `QNetworkAccessManager` et cache. Support des headers personnalisés et authentification Bearer token.
- `ApiClientBase` : classe de base abstraite pour tous les clients API. Fournit parsing JSON générique, gestion d'erreurs standardisée, et méthodes helper pour GET/POST JSON/form.

#### 2.2 Gestion d'erreurs
- `Error` : classe représentant une erreur avec code standardisé, message localisé et contexte optionnel.
- `ErrorHandler` : utilitaires pour créer des erreurs typées (réseau, Twitch, média, validation).
- Tous les services utilisent maintenant `Error` pour une gestion d'erreurs cohérente, avec conversion automatique vers `QString` pour compatibilité QML.

### 3. `media`
- `FFmpegBridge` : encapsulation de l’API C FFmpeg.
- `DecoderPipeline` : gestion des codecs + fallback logiciel.

### 4. `streaming`
- `ManifestParser` : HLS (.m3u8) / DASH (.mpd).
- `SegmentFetcher` : téléchargements parallèles, cache adaptatif.
- `AdaptiveController` : sélection bitrate selon bande passante / charge système.

### 5. `player`
- `MediaSession` : coordination audio/vidéo, tampons, synchronisation.
- `VideoSurfaceRenderer` : rendu GPU via QQuickFramebufferObject.
- `AudioOutput` : intégration QtMultimedia/portaudio (selon besoin).

### 6. `ui`
- `MainView.qml` : commandes principales.
- `QualityOverlay.qml`, `StatsOverlay.qml`, `ChatPanel.qml`.

## Architecture Modulaire

### Dépendances entre modules
- `blueplayer_core` : contient les utilitaires de base (Logger, Error, InputValidator, HttpClient, ApiClientBase) et les services API Twitch.
- `blueplayer_media` : dépend de `blueplayer_core` pour Logger et InputValidator.
- Dépendance circulaire résolue : `Application` (dans `blueplayer_core`) utilise `FFmpegMediaService` (dans `blueplayer_media`) via forward declarations dans le header et inclusion complète dans le `.cpp`.

### Patterns de conception
- **Couche réseau abstraite** : `HttpClient` centralise toutes les requêtes HTTP avec cache et gestion d'erreurs. `ApiClientBase` fournit une base commune pour tous les clients API.
- **Gestion d'erreurs standardisée** : utilisation de `Error` avec codes typés et `ErrorHandler` pour créer des erreurs cohérentes.
- **Forward declarations** : utilisées pour éviter les dépendances circulaires au niveau des headers.
- **Services Qt/QML** : tous les services exposés à QML héritent de `QObject` et utilisent `Q_PROPERTY` et `Q_INVOKABLE`.

## Outils & Build
- `CMakeLists.txt` racine (>=3.24) avec presets Debug/Release.
- Intégration possible via vcpkg/Conan pour FFmpeg et dépendances annexes.
- Scripts utilitaires dans `scripts/` (`build.sh`, `run.sh`, `format.sh`).
- Guide d'installation détaillé : `docs/SETUP.md`.

## Roadmap Technique
1. Initialiser le squelette CMake + configuration Qt. (✅ En cours/partiellement réalisé)
2. Compiler/packager FFmpeg avec accélérations (VideoToolbox, NVDEC, VAAPI). (✅ Dépendances Homebrew trouvées)
3. POC lecture fichier local -> pipeline complet (FFmpeg -> Qt Quick).
4. Implémenter OAuth + client Twitch.
5. Créer StreamFetcher + adaptation.
6. Finaliser player + UI QML.
7. Profilage, packaging multi-plateforme, automatisation QA.
8. Intégrer Twitch Helix (authentification, streams, UI). (en cours/plié)


