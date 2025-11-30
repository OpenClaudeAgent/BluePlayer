# BluePlayer

BluePlayer est une application native de lecture multimédia haute performance, développée en C++ avec le framework Qt et la bibliothèque FFmpeg. L'objectif est de fournir une expérience utilisateur fluide et riche, notamment pour la lecture de contenus vidéo.

## Table des Matières

- [Fonctionnalités](#fonctionnalités)
- [Prérequis](#prérequis)
- [Installation et Compilation](#installation-et-compilation)
- [Exécution](#exécution)
- [Tests](#tests)
- [Structure du Projet](#structure-du-projet)

## Fonctionnalités

- Lecture de divers formats vidéo et audio grâce à FFmpeg.
- Interface utilisateur moderne et réactive développée avec Qt Quick.
- Accélération matérielle pour une performance optimale.

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

Pour des instructions d'installation détaillées et la configuration des variables d'environnement via le fichier `.env`, consultez le document `docs/SETUP.md`.

## Installation et Compilation

Le projet utilise `CMake` pour la configuration et la compilation, orchestrées par un `Makefile` pour simplifier les opérations. Toutes les commandes utilisent le script `./scripts/load_env.sh` pour charger les variables d'environnement depuis le fichier `.env`.

1.  **Clonez le dépôt :**
    ```bash
    git clone <URL_DU_DEPOT>
    cd BluePlayer
    ```

2.  **Créez le fichier `.env` :** À la racine de votre projet, créez un fichier nommé `.env` (il n'est pas versionné par Git). Adaptez les chemins et remplacez `<votre_utilisateur>` par votre nom d'utilisateur réel.
    ```
    BLUEPLAYER_ROOT=$(pwd)
    QT6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6
    FFMPEG_DIR=/opt/homebrew/opt/ffmpeg/lib/cmake/ffmpeg
    PKG_CONFIG_PATH=$FFMPEG_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
    ```

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
