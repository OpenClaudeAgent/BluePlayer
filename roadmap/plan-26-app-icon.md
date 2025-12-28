# Plan 26 - Icône Application BluePlayer

## Contexte
BluePlayer utilise actuellement l'icône par défaut de Qt/macOS dans le dock. Une nouvelle icône personnalisée "Blue Drop Play" a été validée et doit être intégrée à l'application.

### Icône source
- **Fichier** : `assets/icons/01-blue-drop-play.svg`
- **Design** : Goutte d'eau bleue stylisée avec un symbole Play blanc au centre
- **Style** : Fond sombre arrondi, style macOS Big Sur
- **Couleurs** : Gradient bleu (#5bc0ff → #3da2ff → #2878cc), fond (#0b1727 → #040b15)

## Objectif
Remplacer l'icône par défaut par l'icône BluePlayer personnalisée dans :
- Le dock macOS
- Le Finder (icône de l'application)
- La barre de titre de la fenêtre
- Le menu "À propos"

## Comportement attendu

### Dans le Dock
- L'icône BluePlayer apparaît à la place de l'icône Qt générique
- L'icône est nette et bien définie à toutes les tailles (zoom dock)
- L'icône respecte les conventions macOS (coins arrondis, ombre)

### Dans le Finder
- L'icône apparaît correctement dans les différentes vues (icônes, liste, colonnes)
- Le fichier .app affiche la bonne icône
- L'icône est visible dans Spotlight et Launchpad

### Dans l'application
- La barre de titre affiche l'icône (si applicable)
- Le menu "À propos" affiche l'icône de l'application

## Étapes de génération

L'intégration d'une icône macOS nécessite :
1. Conversion du SVG source vers des images PNG à toutes les résolutions requises
2. Génération du fichier .icns (format natif macOS pour les icônes)
3. Configuration du build pour inclure l'icône dans le bundle .app

### Résolutions requises pour macOS
- 16x16, 32x32 (menu, sidebar)
- 128x128, 256x256 (Finder standard)
- 512x512, 1024x1024 (Finder haute résolution, Retina)
- Versions @2x pour les écrans Retina

## Fichiers concernés
- `assets/icons/01-blue-drop-play.svg` (source)
- `src/resources/` (nouveau dossier pour les ressources)
- `CMakeLists.txt` (configuration du bundle)
- Fichier `.icns` généré

## Checklist de validation
- [ ] Dépendance `librsvg` installée (conversion SVG → PNG)
- [ ] PNG générés à toutes les résolutions (16 à 1024px)
- [ ] Fichier .icns créé avec `iconutil`
- [ ] Dossier `src/resources/` créé avec l'icône
- [ ] CMakeLists.txt configuré pour le bundle macOS
- [ ] Build réussi sans erreur
- [ ] Icône visible dans le Dock après lancement
- [ ] Icône nette à toutes les tailles de dock (zoom)
- [ ] Icône visible dans le Finder pour BluePlayer.app
- [ ] Icône visible dans Spotlight
- [ ] Pas de régression sur le comportement de l'application
