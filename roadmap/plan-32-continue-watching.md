# Plan 32 - Section "Continuer à regarder"

## Contexte

Le CacheManagerView permet de sauvegarder des replays (VOD) avec leur progression de lecture. L'utilisateur souhaite un accès rapide à ces VOD depuis la HomeView, sans devoir naviguer vers le gestionnaire de cache.

## Objectif

Ajouter une nouvelle ligne dans la HomeView (en 2ème ou 3ème position) qui affiche les replays sauvegardés dans le cache. C'est un raccourci vers les fonctionnalités du CacheManagerView.

## Spécifications

### Comportement attendu

1. **Affichage des VOD cachées**
   - Affiche les VOD/replays sauvegardés dans le cache
   - Chaque carte montre la progression de lecture (barre de progression ou pourcentage)
   - Les VOD sont triées par date de dernier visionnage (le plus récent en premier)

2. **Reprise de la lecture**
   - Un clic sur une carte lance directement la lecture depuis la position sauvegardée
   - L'utilisateur reprend là où il s'était arrêté

3. **Gestion du cas vide**
   - Si aucun replay n'est caché, la section ne s'affiche pas
   - Alternative : afficher un message invitant à sauvegarder des VOD

### Position dans la Home

- La section apparaît en 2ème ou 3ème position (à définir lors de l'implémentation)
- Suggestion : après "Vos streamers suivis", avant "Parcourir"

### Design

- Utilise le même style de cartes que le CacheManagerView
- Indicateur visuel de progression (barre sous la miniature ou overlay)
- Titre de section : "Continuer à regarder" / "Continue Watching"

## Fichiers concernés

- `src/ui/HomeView.qml` - Ajout de la nouvelle section
- `src/ui/HomeViewModel.cpp` - Récupération des VOD cachées
- `src/ui/HomeViewModel.hpp` - Propriété pour les VOD cachées
- `src/ui/CacheManagerViewModel.cpp` - Réutilisation de la logique existante (si applicable)

## Checklist de validation

### Affichage
- [ ] La section "Continuer à regarder" apparaît dans la Home
- [ ] Les VOD cachées s'affichent avec leur miniature
- [ ] La progression de lecture est visible sur chaque carte
- [ ] Les VOD sont triées par date de dernier visionnage

### Interaction
- [ ] Un clic sur une VOD lance la lecture
- [ ] La lecture reprend à la position sauvegardée
- [ ] Navigation fluide vers le player

### Gestion du vide
- [ ] La section est masquée si aucune VOD n'est cachée
- [ ] (Alternative) Un message d'incitation s'affiche

### Non-régression
- [ ] Les autres sections de la Home fonctionnent normalement
- [ ] Le CacheManagerView fonctionne toujours correctement
- [ ] L'application compile sans erreur
