# BluePlayer – Architecture de Référence

## Objectifs
- Lecteur Twitch natif, focalisé sur la performance et la faible latence.
- Exploiter C++/Qt pour l’UI et la gestion système, FFmpeg pour le décodage matériel.
- Fournir une base modulaire facilitant les évolutions (chat, overlay, instrumentation).

## Vue d’Ensemble
```
src/
  api/twitch        -> Authentification OAuth2 + récupération des manifestes HLS/DASH
  core/             -> Configuration, télémétrie, utilitaires
    media           -> Bridges FFmpeg, gestion des codecs/accélérations
    system          -> Monitoring CPU/GPU, préférences utilisateur
  media/            -> Sessions média, pipelines de décodage
  streaming/        -> Fetcher HLS/DASH, adaptation de qualité, cache segments
  player/           -> Orchestrateur Audio/Vidéo, synchronisation, rendu GPU
  ui/               -> Composants QML, commandes playback, statistiques
```

## Modules

### 1. `api/twitch`
- `TwitchAuthManager` : flux OAuth (PKCE), stockage sécurisé des tokens.
- `TwitchApiClient` : accès aux endpoints (streams live, VOD, manifestes).
- `docs/twitch-api.md` décrira les endpoints utilisés et les contraintes de quota.

### 2. `core`
- `SettingsStore` : persistance JSON/INI.
- `SystemMonitor` : collecte CPU, GPU, réseau.
- `TelemetryLogger` : traces pour le profilage.

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

## Outils & Build
- `CMakeLists.txt` racine (>=3.24) avec presets Debug/Release.
- Intégration possible via vcpkg/Conan pour FFmpeg et dépendances annexes.
- Scripts utilitaires dans `scripts/` (`build.sh`, `run.sh`, `format.sh`).
- Guide d’installation détaillé : `docs/SETUP.md`.

## Roadmap Technique
1. Initialiser le squelette CMake + configuration Qt.
2. Compiler/packager FFmpeg avec accélérations (VideoToolbox, NVDEC, VAAPI).
3. POC lecture fichier local -> pipeline complet (FFmpeg -> Qt Quick).
4. Implémenter OAuth + client Twitch.
5. Créer StreamFetcher + adaptation.
6. Finaliser player + UI QML.
7. Profilage, packaging multi-plateforme, automatisation QA.


