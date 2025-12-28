# Plan 39 - Export des VOD cachées

## Contexte

BluePlayer permet de cacher (enregistrer) des VOD localement pour une lecture hors-ligne. Ces fichiers sont stockés dans un format interne. L'utilisateur souhaite pouvoir exporter ces VOD vers un fichier vidéo standard (MP4) pour les conserver, les partager ou les éditer dans un logiciel externe.

## Objectif

Permettre d'exporter les VOD du cache vers un fichier vidéo au format MP4, avec choix du dossier de destination et suivi de la progression.

## Specifications

### Comportement attendu

1. **Accès à l'export**
   - Bouton "Exporter" sur chaque VOD dans le CacheManagerView
   - Menu contextuel alternatif (clic droit)
   - Icône reconnaissable (flèche vers le bas, icône export)

2. **Sélection de la destination**
   - Dialogue système pour choisir le dossier de destination
   - Nom de fichier par défaut : titre de la VOD + date
   - Possibilité de renommer le fichier

3. **Options d'export**
   - Choix de la qualité : Originale, Haute, Moyenne, Basse
   - Estimation de la taille du fichier final
   - Format : MP4 (H.264 + AAC) pour compatibilité maximale

4. **Progression**
   - Barre de progression pendant l'export
   - Pourcentage et temps restant estimé
   - Possibilité d'annuler l'export en cours

5. **Finalisation**
   - Notification de succès avec lien vers le fichier
   - Option "Révéler dans le Finder"
   - Gestion des erreurs (espace disque, permissions)

### Considérations techniques

- Utiliser FFmpeg pour le transcodage
- L'export doit se faire en arrière-plan (non bloquant)
- Gérer les VOD partiellement téléchargées

## Fichiers concernes

- `src/ui/CacheManagerView.qml` - Bouton export et UI
- Nouveau : `src/core/VideoExporter.cpp/.hpp` - Logique d'export
- `src/core/CacheManager.cpp` - Accès aux fichiers cachés

## Checklist de validation

### Interface
- [ ] Le bouton "Exporter" est visible sur chaque VOD cachée
- [ ] Le dialogue de sélection de destination s'ouvre
- [ ] Les options de qualité sont proposées

### Export
- [ ] L'export génère un fichier MP4 valide
- [ ] La qualité correspond à l'option choisie
- [ ] L'export en arrière-plan ne bloque pas l'interface

### Progression
- [ ] La barre de progression s'affiche et se met à jour
- [ ] Le temps restant estimé est affiché
- [ ] L'annulation de l'export fonctionne

### Finalisation
- [ ] La notification de succès s'affiche
- [ ] Le bouton "Révéler dans le Finder" fonctionne
- [ ] Les erreurs sont affichées clairement

### Non-régression
- [ ] Le CacheManagerView fonctionne normalement
- [ ] Les VOD cachées sont toujours lisibles
- [ ] L'application compile sans erreur
