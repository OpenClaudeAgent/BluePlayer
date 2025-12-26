# Plan 07 - Barre de recherche Twitch

## Contexte
La barre de recherche est implémentée au niveau UI uniquement. Elle n'est pas connectée à l'API Twitch et ne retourne aucun résultat.

## Objectif
Implémenter une barre de recherche fonctionnelle qui interroge l'API Twitch et affiche les résultats en temps réel.

## Spécifications

### Recherche
- Recherche déclenchée à la frappe (avec debounce ~400ms)
- Appel à l'API Twitch Search
- Support des différents types de résultats :
  - Chaînes (streamers)
  - Catégories/Jeux
  - Streams en direct

### Affichage des résultats
- Liste déroulante sous la barre de recherche
- Affichage des résultats par catégorie (Chaînes, Jeux, Live)
- Thumbnail/avatar pour chaque résultat
- Indication si le streamer est en live

### Interactions
- Clic sur un streamer → ouvre le player avec son stream (si live) ou sa page
- Clic sur une catégorie → affiche les streams de cette catégorie
- Navigation au clavier (flèches haut/bas, Entrée)
- Fermeture de la liste quand on clique ailleurs

### API Twitch concernées
- `GET /helix/search/channels` - Recherche de chaînes
- `GET /helix/search/categories` - Recherche de catégories
- `GET /helix/streams` - Pour vérifier si un streamer est live

## Fichiers concernés
- `src/ui/HomeView.qml` (barre de recherche UI)
- `src/ui/HomeViewModel.cpp` (logique de recherche)
- `src/api/twitch/TwitchApiClient.cpp` (appels API)
- `src/api/twitch/TwitchService.cpp` (orchestration)
- Nouveau : `src/ui/components/SearchResults.qml` (affichage résultats)

## Checklist de validation
- [ ] La recherche se déclenche à la frappe (avec debounce)
- [ ] L'API Twitch search/channels est appelée
- [ ] L'API Twitch search/categories est appelée
- [ ] Les résultats de chaînes s'affichent
- [ ] Les résultats de catégories s'affichent
- [ ] Les streams en direct sont indiqués (badge live)
- [ ] Clic sur un streamer live ouvre le player
- [ ] Clic sur une catégorie affiche les streams de cette catégorie
- [ ] Navigation au clavier fonctionne (flèches, Entrée)
- [ ] Les thumbnails/avatars s'affichent
- [ ] La liste de résultats se ferme quand on clique ailleurs
- [ ] Gestion du cas "pas de résultat"
- [ ] Gestion des erreurs réseau
- [ ] Performances acceptables (pas de lag à la frappe)
