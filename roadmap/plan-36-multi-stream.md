# Plan 36 - Multi-stream

## Contexte

Les utilisateurs veulent parfois regarder plusieurs streams en même temps (tournois, événements multi-POV, streams d'amis). Actuellement, il faut ouvrir plusieurs instances de l'application ou utiliser un service web externe.

## Objectif

Permettre d'afficher 2 à 4 streams simultanément dans une grille, avec la possibilité de choisir quel stream a l'audio actif.

## Specifications

### Comportement attendu

1. **Activation du mode multi-stream**
   - Option pour ajouter un stream à la grille depuis la Home ou la recherche
   - Bouton "Ajouter à la grille" sur les cartes de stream
   - Alternative : raccourci clavier pour ajouter le stream sélectionné

2. **Affichage en grille**
   - Grille simple : 2 streams (côte à côte) ou 4 streams (2x2)
   - Chaque cellule contient un player minimal (vidéo + infos essentielles)
   - La grille s'adapte automatiquement au nombre de streams

3. **Gestion de l'audio**
   - Un seul stream a l'audio actif à la fois
   - Indicateur visuel du stream avec audio (bordure, icône)
   - Clic sur un stream pour lui donner l'audio
   - Les autres streams sont en mute

4. **Contrôles par stream**
   - Chaque stream a ses propres contrôles basiques (play/pause, volume si actif)
   - Bouton pour retirer un stream de la grille
   - Bouton pour mettre un stream en plein écran (quitte le mode multi)

5. **Sortie du mode multi-stream**
   - Fermer tous les streams sauf un revient au mode normal
   - Option "Quitter le multi-stream" dans le menu

### Limitations V1

- Maximum 4 streams simultanés
- Layout fixe (pas de redimensionnement des cellules)
- Pas de chat en mode multi-stream (ou un seul chat visible)
- Pas de support VOD (streams live uniquement)

## Fichiers concernes

- Nouveau : `src/ui/MultiStreamView.qml` - Vue grille multi-stream
- Nouveau : `src/ui/MultiStreamViewModel.cpp/.hpp` - Logique de gestion
- `src/ui/components/StreamCard.qml` - Ajout option "Ajouter à la grille"
- `src/media/MpvQuickItem.cpp` - Gestion de plusieurs instances player

## Checklist de validation

### Ajout de streams
- [ ] Le bouton "Ajouter à la grille" apparaît sur les cartes
- [ ] Cliquer ajoute le stream à la grille
- [ ] La limite de 4 streams est respectée

### Affichage grille
- [ ] 2 streams s'affichent côte à côte
- [ ] 4 streams s'affichent en grille 2x2
- [ ] Chaque stream est visible et lisible

### Gestion audio
- [ ] Un seul stream a l'audio à la fois
- [ ] L'indicateur visuel identifie le stream avec audio
- [ ] Cliquer sur un stream lui donne l'audio

### Contrôles
- [ ] Chaque stream peut être retiré de la grille
- [ ] Le mode plein écran fonctionne sur un stream
- [ ] La sortie du mode multi-stream est possible

### Non-régression
- [ ] Le mode player normal fonctionne
- [ ] Les performances restent acceptables avec 4 streams
- [ ] L'application compile sans erreur
