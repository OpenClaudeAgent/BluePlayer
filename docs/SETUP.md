# BluePlayer – Guide d’installation des outils

Ce document décrit comment préparer un environnement de développement local performant pour BluePlayer sur macOS (Apple Silicon ou Intel). Adaptez les commandes si vous travaillez sous Linux ou Windows.

## 1. Prérequis système

1. **Xcode Command Line Tools**  
   ```bash
   xcode-select --install
   ```
2. **Homebrew (optionnel mais recommandé)**  
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

## 2. Outils de build

```bash
brew install cmake ninja git python@3.11
```

- `cmake` ≥ 3.24 pour la configuration multi-plateforme.  
- `ninja` pour des builds rapides (utilisé dans les presets CMake recommandés).  
- `python` est requis si vous utilisez `conan` ou des scripts auxiliaires.

## 3. Qt 6

Deux options :

### a. Qt via Homebrew
```bash
brew install qt@6
echo 'export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"' >> ~/.zshrc
```

### b. Qt Online Installer
1. Télécharger l’installateur depuis https://www.qt.io/download.
2. Installer Qt 6.5+ avec Qt Creator et **Qt Quick**/**Qt Multimedia**.

> Vérifiez avec `qmake -v` ou `qtpaths --qt-version`.

## 4. Gestionnaire de dépendances (optionnel)

Vous pouvez utiliser **Conan** ou **vcpkg** pour récupérer FFmpeg et d’autres libs.

- Conan :
  ```bash
  pip3 install conan
  conan profile detect --force
  ```
- vcpkg :
  ```bash
  git clone https://github.com/microsoft/vcpkg.git
  ./vcpkg/bootstrap-vcpkg.sh
  ```

## 5. FFmpeg avec accélération matérielle

BluePlayer nécessite FFmpeg compilé avec :

- `--enable-gpl --enable-nonfree`
- `--enable-libdav1d --enable-libvpx --enable-libx264 --enable-libx265`
- Accélérations macOS : `--enable-videotoolbox`

### Intégration de FFmpeg avec CMake

Pour que CMake puisse trouver FFmpeg, vous devez définir la variable d'environnement `FFmpeg_DIR` qui pointe vers le répertoire d'installation de FFmpeg.

**Option A : Installation rapide (Homebrew + frameworks)**
```bash
brew install ffmpeg --with-srt --with-webp --with-opus --with-videotoolbox
export FFmpeg_DIR="$(brew --prefix)/opt/ffmpeg/lib/cmake/ffmpeg" # Nécessaire si CMake ne trouve pas FFmpeg automatiquement
```
> Vérifiez avec `ffmpeg -hwaccels`.

**Option B : Compilation personnalisée**
```bash
git clone https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg
./configure --prefix=$HOME/libs/ffmpeg \
            --enable-gpl --enable-nonfree \
            --enable-videotoolbox \
            --enable-libx264 --enable-libx265 --enable-libvpx \
            --enable-libopus --enable-libdav1d
make -j$(sysctl -n hw.logicalcpu)
make install
```
Puis, ajoutez à votre `~/.zshrc` ou `~/.bashrc` :
```bash
export FFmpeg_DIR="$HOME/libs/ffmpeg/lib/cmake/ffmpeg" # Assurez-vous que ce chemin est correct pour votre installation
export PKG_CONFIG_PATH="$FFmpeg_DIR/lib/pkgconfig:$PKG_CONFIG_PATH"
```

## 6. Gestion des variables d'environnement avec `.env`

Pour gérer les variables d'environnement spécifiques au projet sans modifier vos fichiers de configuration shell globaux (`~/.zshrc`, `~/.bashrc`), BluePlayer utilise un fichier `.env` et un script `load_env.sh`.

1.  **Créez le fichier `.env` :** À la racine de votre projet BluePlayer, créez un fichier nommé `.env` (s'il n'existe pas déjà) avec le contenu suivant. Ce fichier est généralement optionnel si Homebrew gère la plupart de vos dépendances, mais il peut être utile pour des configurations spécifiques ou des chemins non standards.

    ```
    BLUEPLAYER_ROOT=$(pwd)
    # Si nécessaire, ajoutez d'autres variables ici, par exemple :
    # QT6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6
    # FFMPEG_DIR=/opt/homebrew/opt/ffmpeg/lib/cmake/ffmpeg
    # PKG_CONFIG_PATH=$FFMPEG_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
    ```

2.  **Script de chargement :** Un script `scripts/load_env.sh` est fourni pour charger ces variables et exécuter vos commandes. Il est automatiquement créé et rendu exécutable lors de la configuration initiale.

> **Important :** Le fichier `.env` ne doit pas être versionné (il est ignoré par Git) car il peut contenir des chemins spécifiques à votre machine.

## 7. Utilisation du Makefile

Le projet BluePlayer inclut un `Makefile` à sa racine pour simplifier les opérations courantes. Toutes les commandes du `Makefile` utilisent le script `scripts/load_env.sh` et donc les variables définies dans `.env`.

### 7.1. Configuration et Compilation

Pour configurer et compiler le projet :

```bash
make build
```

### 7.2. Exécution de BluePlayer

Après une compilation réussie, lancez l'application avec :

```bash
make run
```

### 7.3. Exécution des Tests

Pour exécuter tous les tests :

```bash
make test
```

### 7.4. Autres commandes

-   `make clean` : Nettoie le répertoire de build.
-   `make help`  : Affiche toutes les cibles disponibles et leur description.

> Vous pouvez également lancer l'application via Qt Creator si vous l'avez installé.

## 8. Stratégie de tests

Le projet BluePlayer utilise [CTest](https://cmake.org/cmake/help/latest/module/CTest.html) pour l'exécution des tests.

### 8.1. Types de tests

1.  **Tests Unitaires :** Pour valider le bon fonctionnement des composants individuels (ex: `FFmpegBridge`, `Application`). Il est recommandé d'utiliser un framework tel que [Google Test](https://github.com/google/googletest) ou [Catch2](https://github.com/catchorg/Catch2).
2.  **Tests d'Intégration :** Pour vérifier l'interaction entre les différents modules du projet (ex: lecture vidéo avec l'interface utilisateur). Ces tests assurent que les composants fonctionnent ensemble comme prévu.

## 9. Intégration Twitch

1.  **Créer une application Twitch**  
    Rendez-vous dans la [console développeur Twitch](https://dev.twitch.tv/console/apps), créez une nouvelle application et configurez une URL de redirection locale (ex: `http://127.0.0.1:45111/callback`).

2.  **Variables d'environnement**  
    ```
    TWITCH_CLIENT_ID=<votre client id>
    TWITCH_REDIRECT_URI=http://127.0.0.1:45111/callback
    TWITCH_REDIRECT_PORT=45111
    TWITCH_CLIENT_SECRET=<facultatif, utile pour les appels privés>
    ```
    Le flux OAuth est géré avec PKCE : l'application génère un `code_challenge`, ouvre l'URL d'autorisation et échange le `code` reçu pour un jeton via un serveur local.

3.  **Scope & stockage**  
    Le service demande `user:read:email user:read:follows`. Les jetons sont persistés dans `QSettings` (`BluePlayer/Twitch`) pour éviter de relancer la connexion à chaque démarrage. Utilisez le bouton « Déconnexion » pour forcer un nouveau flux OAuth.

4.  **Guidage dans l'UI**  
    L'interface expose un panneau Twitch avec les boutons Connexion / Déconnexion / Actualiser et une liste des streams. La sélection d'un stream ouvre l'URL Twitch dans le navigateur.

