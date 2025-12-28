# BluePlayer

> **Note** : Ce projet est développé en mode *vibe-coding* avec Claude/OpenCode. C'est un projet expérimental qui sert également de terrain d'exploration pour le développement assisté par IA.

BluePlayer est une application native de lecture multimédia haute performance, développée en C++ avec le framework Qt et la bibliothèque FFmpeg. L'objectif est de fournir une expérience utilisateur fluide et riche, notamment pour la lecture de contenus vidéo.

## Table des Matières

- [Fonctionnalités](#fonctionnalités)
- [Documentation](#documentation)
- [Prérequis](#prérequis)
- [Installation et Compilation](#installation-et-compilation)
- [Exécution](#exécution)
- [Tests](#tests)
- [Structure du Projet](#structure-du-projet)
- [Roadmap](#roadmap)

## Fonctionnalités

- Lecture de divers formats vidéo et audio grâce à FFmpeg.
- Interface utilisateur moderne et réactive développée avec Qt Quick.
- Accélération matérielle pour une performance optimale.

## Documentation

| Document | Description |
|----------|-------------|
| [SETUP.md](docs/SETUP.md) | Guide d'installation détaillé et configuration |
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | Architecture technique du projet |
| [AGENTIC-WORKFLOW.md](docs/AGENTIC-WORKFLOW.md) | Workflow des agents IA (vibe-coding) |

## Prérequis

Assurez-vous d'avoir les éléments suivants installés sur votre système :

-   **Xcode Command Line Tools** (macOS) : `xcode-select --install`
-   **Homebrew** (optionnel mais recommandé pour macOS) : Installez-le via le site officiel.
-   **CMake** (version ≥ 3.24)
-   **Ninja** (pour des builds rapides)
-   **Python 3.11+** (si vous utilisez Conan ou vcpkg)
-   **Qt 6.5+** (avec les modules Qt Quick et Qt Multimedia).
    -   Via Homebrew : `brew install qt@6` puis ajoutez `export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"` à votre `~/.zshrc` ou `~/.bashrc`.
    -   Via Qt Online Installer : Téléchargez l'installateur depuis [qt.io](https://www.qt.io/download).
-   **FFmpeg** (compilé avec `--enable-gpl --enable-nonfree`, `--enable-libdav1d --enable-libvpx --enable-libx264 --enable-libx265`, et `--enable-videotoolbox` pour macOS).
    -   Via Homebrew : `brew install ffmpeg --with-srt --with-webp --with-opus --with-videotoolbox`
    -   Assurez-vous que la variable d'environnement `FFmpeg_DIR` pointe vers l'emplacement des fichiers de configuration CMake de FFmpeg (par exemple, `$(brew --prefix)/opt/ffmpeg/lib/cmake/ffmpeg` pour Homebrew ou `$HOME/libs/ffmpeg/lib/cmake/ffmpeg` pour une compilation personnalisée).
-   **Compte développeur Twitch** (créez une application dans la console Twitch, configurez une redirection locale et définissez `TWITCH_CLIENT_ID`, `TWITCH_REDIRECT_URI` et `TWITCH_REDIRECT_PORT`).
-   **TLS local (mkcert)** : installez `mkcert`, générez des certificats pour `localhost` ou `127.0.0.1`, puis exportez `TWITCH_TLS_CERT_PATH` et `TWITCH_TLS_KEY_PATH` pour que le serveur OAuth écoute en HTTPS.

Pour des instructions d'installation détaillées et la configuration des variables d'environnement via le fichier `.env`, consultez le document `docs/SETUP.md`.

## Installation et Compilation

Le projet utilise `CMake` pour la configuration et la compilation, orchestrées par un `Makefile` pour simplifier les opérations. Toutes les commandes utilisent le script `./scripts/load_env.sh` pour charger les variables d'environnement depuis le fichier `.env`.

1.  **Clonez le dépôt :**
    ```bash
    git clone https://github.com/<votre-utilisateur>/BluePlayer.git # Remplacez par l'URL de votre dépôt
    cd BluePlayer
    ```

2.  **Configurez les variables d'environnement (si nécessaire) :**
    Si vous avez des installations personnalisées de Qt ou FFmpeg, vous devrez peut-être définir les chemins d'accès.
    Pour Homebrew sur macOS, CMake devrait trouver les bibliothèques automatiquement.
    Si vous rencontrez des problèmes ou que vous voulez connecter l’application à Twitch, créez un fichier `.env` à la racine de votre projet avec les variables nécessaires (`FFmpeg_DIR`, `Qt6_DIR`, `TWITCH_CLIENT_ID`, `TWITCH_REDIRECT_URI`, `TWITCH_REDIRECT_PORT`, etc.).

3.  **Configurez et compilez le projet :**
    Utilisez la commande `make` à la racine du projet.
    ```bash
    make build
    ```

## Exécution

Pour exécuter l'application après une compilation réussie :

```bash
make run
```

## Tests

Pour exécuter les tests (unitaires et d'intégration) après la compilation :

```bash
make test
```

## Autres commandes Make

-   `make clean` : Nettoie le répertoire de build.
-   `make help`  : Affiche toutes les cibles disponibles et leur description.

## Structure du Projet

-   `cmake/` : Fichiers CMake auxiliaires.
-   `docs/` : Documentation du projet (SETUP, ARCHITECTURE, etc.).
-   `scripts/` : Scripts utilitaires (ex: `load_env.sh`).
-   `src/` : Code source principal de l'application.
    -   `api/` : Interfaces avec des services externes (ex: Twitch).
    -   `core/` : Logique applicative principale.
    -   `media/` : Gestion de la lecture multimédia avec FFmpeg.
    -   `player/` : Composants du lecteur vidéo.
    -   `streaming/` : Gestion du streaming.
    -   `ui/` : Interface utilisateur QML.
-   `tests/` : Tests unitaires et d'intégration.

## Roadmap

Consultez la [Roadmap](roadmap/README.md) pour le suivi des tâches planifiées et en cours.

## Changelog

| Version | Date | Description |
|---------|------|-------------|
| v0.20.0 | 2025-12-28 | Persistance qualité par défaut (sauvegarde préférences, application auto au stream, fallback intelligent) |
| v0.19.0 | 2025-12-28 | Panel Préférences modernisé (design, dropdown qualité, cache, composant BlueDropdown) |
| v0.18.0 | 2025-12-28 | Sélecteur qualité stream (bouton HD/SD dynamique, popup, qualité VOD cachée) |
| v0.17.0 | 2025-12-28 | Tests Plan 24 (+32 tests unitaires et intégration) - **Plan 24 TERMINÉ** |
| v0.16.0 | 2025-12-28 | Refactoring Media (downloadThumbnail asynchrone, thread UI non bloqué) |
| v0.15.0 | 2025-12-28 | Refactoring TwitchService (helper ensureTokenAndExecute, -29 lignes) |
| v0.14.0 | 2025-12-28 | Refactoring Cards (BaseCard créé, 5 cards refactorées, -239 lignes) |
| v0.13.0 | 2025-12-27 | Refactoring PlayerControlBar (ControlButton, SeekBar, VolumeControl) + signature code automatique |
| v0.12.0 | 2025-12-27 | Refactoring PlayerView (composants QML extraits, logique recording en C++, injection dépendances) |
| v0.11.0 | 2025-12-27 | Sécurité renforcée (Keychain macOS, validation IRC, thread-safety, logging) |
| v0.10.0 | 2025-12-27 | Animations harmonisées, transitions navigation fluides, architecture vues optimisée |
| v0.9.0 | 2025-12-27 | Vitesse lecture intelligente (auto-reset au live, protection stutter, mode replay) |
| v0.8.0 | 2025-12-27 | Fix Home : suppression section redondante "En direct maintenant" |
| v0.7.0 | 2025-12-27 | Chat Twitch intégré (lecture, envoi, emotes, badges) |
| v0.6.0 | 2025-12-27 | Barre de recherche Twitch (channels live + cache local) |
| v0.5.0 | 2025-12-26 | Gestion Cache/VOD (enregistrement auto, thumbnails, mode replay) |
| v0.4.0 | 2025-12-26 | Layout barre de contrôle réorganisé + timer intelligent |
| v0.3.0 | 2025-12-26 | Largeur boutons harmonisée (64px) + style uniforme |
| v0.2.0 | 2025-12-26 | Fullscreen harmonisé (comportement unifié + curseur auto-caché) |
| v0.1.0 | 2025-12-26 | Bouton Volume (slider vertical + hover) |
