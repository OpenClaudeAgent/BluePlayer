# BluePlayer – Architecture de Référence

## Objectifs
- Lecteur Twitch natif, focalisé sur la performance et la faible latence.
- Exploiter C++/Qt pour l’UI et la gestion système, FFmpeg pour le décodage matériel.
- Fournir une base modulaire facilitant les évolutions (chat, overlay, instrumentation).

## Vue d'Ensemble
```
src/
  api/twitch/       -> Authentification OAuth2, API Helix, service Qt exposé à QML
  chat/             -> Client IRC Twitch pour le chat en direct
  core/             -> Configuration, logging, gestion d'erreurs, cache, stockage sécurisé
    network/        -> Couche réseau abstraite (HttpClient, ApiClientBase)
  media/            -> Intégration MPV, filtrage publicités HLS, contrôle vitesse
  ui/               -> Vues QML, ViewModels, composants réutilisables
    components/     -> PlayerControlBar, Cards, boutons, overlays
    themes/         -> Thèmes visuels (BlueTheme.js)
```

## Modules

### 1. `api/twitch`
- `TwitchAuthManager` : flux OAuth (PKCE), stockage sécurisé des tokens, serveur de callback local TLS (mkcert) pour les redirections HTTPS. Utilise `HttpClient` pour les requêtes réseau.
- `TwitchApiClient` : accès aux endpoints (streams live, VOD, manifestes). Hérite de `ApiClientBase` pour la gestion réseau centralisée.
- `TwitchService` : service Qt exposant l'état d'authentification, la liste des streams et la sélection de VOD vers l'UI QML.

### 2. `core`
- `Config` : configuration centralisée depuis variables d'environnement et fichiers.
- `Logger` / `FileLogger` : système de logging structuré avec catégories et persistance fichier.
- `Error` / `ErrorHandler` : gestion d'erreurs standardisée avec codes d'erreur typés.
- `InputValidator` : validation d'entrées utilisateur (URLs, chemins, tokens).
- `SecureStorage` : stockage sécurisé des données sensibles (tokens OAuth).
- `NetworkCache` : cache réseau avec TTL configurable.
- `CacheManager` : gestion du cache des VODs pour lecture hors-ligne.
- `WatchHistory` : historique de visionnage persistant avec positions de reprise.
- `StateMachineLiveReplay` : machine d'état pour la gestion live/replay.
- `LanguageManager` : gestion de l'internationalisation (i18n) avec changement à chaud.
- `Application` : point d'entrée principal, orchestre les services.

#### 2.1 `core/network` (Nouveau)
- `HttpClient` : client HTTP centralisé avec gestion de `QNetworkAccessManager` et cache. Support des headers personnalisés et authentification Bearer token.
- `ApiClientBase` : classe de base abstraite pour tous les clients API. Fournit parsing JSON générique, gestion d'erreurs standardisée, et méthodes helper pour GET/POST JSON/form.

#### 2.2 Gestion d'erreurs
- `Error` : classe représentant une erreur avec code standardisé, message localisé et contexte optionnel.
- `ErrorHandler` : utilitaires pour créer des erreurs typées (réseau, Twitch, média, validation).
- Tous les services utilisent maintenant `Error` pour une gestion d'erreurs cohérente, avec conversion automatique vers `QString` pour compatibilité QML.

### 3. `chat`
- `TwitchChatClient` : client IRC pour la connexion au chat Twitch. Gère l'authentification, l'envoi/réception de messages et le parsing des commandes IRC.

### 4. `media`
- `MpvQuickItem` : intégration du lecteur MPV dans Qt Quick via rendu OpenGL direct.
- `HlsAdFilter` : filtrage des segments publicitaires dans les flux HLS Twitch.
- `PlaybackSpeedLogic` : gestion intelligente de la vitesse de lecture (0.25x à 2x) avec auto-reset au live.

### 5. `ui`
- `main.qml` : point d'entrée QML, navigation entre vues.
- `HomeView.qml` + `HomeViewModel` : liste des streams et catégories.
- `PlayerView.qml` + `VideoPlayer.qml` : lecteur vidéo avec contrôles.
- `LoginView.qml` : authentification OAuth Twitch.
- `PreferencesView.qml` : paramètres utilisateur.
- `CacheManagerView.qml` + `CacheManagerViewModel` : gestion du cache VOD.

#### 5.1 `ui/components`
- `PlayerControlBar.qml` : barre de contrôle du lecteur (play/pause, volume, qualité).
- `BaseCard.qml`, `StreamCard.qml`, `GameCard.qml`, `VodCard.qml` : cartes pour afficher streams, jeux et VODs.
- `FollowButton.qml`, `FullscreenButton.qml`, `VolumeButton.qml` : boutons réutilisables.
- `QualitySelector.qml`, `PlaybackSpeedSelector.qml` : sélecteurs de qualité et vitesse.

#### 5.2 `ui/themes`
- `BlueTheme.js` : définition des couleurs, polices et espacements du thème.

## Architecture Modulaire

### Dépendances entre modules
- `blueplayer_core` : contient les utilitaires de base (Logger, Error, InputValidator, HttpClient, ApiClientBase), les services API Twitch et le client chat.
- `blueplayer_media` : dépend de `blueplayer_core` pour Logger et InputValidator. Intègre MPV pour la lecture vidéo.

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

## État du Projet

Le projet est en développement actif. Consultez la [Roadmap](../roadmap/README.md) pour le suivi des fonctionnalités planifiées et livrées.


