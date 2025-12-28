# Plan 38 - Mode Debug

## Contexte

Lorsqu'un problème survient (buffering, qualité dégradée, déconnexions), il est difficile de diagnostiquer la cause sans informations techniques. Les utilisateurs et développeurs ont besoin d'accéder à des métriques de lecture pour comprendre et résoudre les problèmes.

## Objectif

Ajouter une option "Mode Debug" dans les préférences qui affiche des informations techniques détaillées sur la lecture en cours.

## Specifications

### Comportement attendu

1. **Activation dans les Préférences**
   - Toggle "Mode Debug" dans le panel Préférences
   - Désactivé par défaut
   - Prend effet immédiatement sans redémarrage

2. **Overlay d'informations (optionnel)**
   - Affichage semi-transparent sur la vidéo
   - Position : coin supérieur gauche ou droit
   - Ne gêne pas le visionnage (petit, discret)

3. **Informations affichées**
   - Bitrate actuel (Mbps)
   - Résolution du stream (ex: 1920x1080)
   - FPS (images par seconde)
   - Dropped frames (images perdues)
   - Buffer size (secondes de buffer)
   - Latency (décalage avec le live)
   - Codec vidéo/audio utilisé

4. **Logs détaillés**
   - Écriture de logs techniques dans la console
   - Optionnel : export vers un fichier log
   - Informations de connexion, erreurs réseau, etc.

5. **Performance**
   - L'overlay ne doit pas impacter les performances
   - Mise à jour des métriques toutes les secondes (pas en temps réel)

## Fichiers concernes

- `src/ui/PreferencesView.qml` - Toggle Mode Debug
- Nouveau : `src/ui/components/DebugOverlay.qml` - Overlay d'informations
- `src/media/MpvQuickItem.cpp` - Extraction des métriques
- `src/core/Config.cpp` - Persistance du réglage

## Checklist de validation

### Activation
- [ ] Le toggle est présent dans les Préférences
- [ ] Le mode s'active/désactive sans redémarrage
- [ ] Le réglage est persisté entre les sessions

### Overlay
- [ ] L'overlay s'affiche sur la vidéo quand activé
- [ ] Les informations sont lisibles mais discrètes
- [ ] L'overlay disparaît quand le mode est désactivé

### Métriques
- [ ] Le bitrate s'affiche et se met à jour
- [ ] La résolution est correcte
- [ ] Les FPS sont affichés
- [ ] Les dropped frames sont comptabilisés
- [ ] La latence est mesurée (pour les streams live)

### Logs
- [ ] Les logs détaillés apparaissent dans la console
- [ ] Les informations sont utiles pour le diagnostic

### Non-régression
- [ ] Le player fonctionne normalement avec le mode activé
- [ ] Pas d'impact notable sur les performances
- [ ] L'application compile sans erreur
