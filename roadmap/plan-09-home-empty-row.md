# Plan 09 - Fix ligne vide Home Page

## Contexte
Dans la page d'accueil (Home), une ligne est vide :
- **Ligne concernée** : "En direct maintenant" / "Les streams les plus populaires"
- **Position** : 4ème ligne (avant-dernière)
- **Symptôme** : La ligne s'affiche mais ne contient aucun contenu

## Objectif
Identifier et corriger la cause de la ligne vide pour afficher les streams populaires.

## Investigation préliminaire

### Questions à résoudre
1. L'API est-elle appelée pour cette section ?
2. L'API retourne-t-elle des données ?
3. Les données sont-elles correctement transformées ?
4. Les données arrivent-elles jusqu'au QML ?
5. Le composant QML affiche-t-il les données ?

### Points de vérification
- `TwitchService` : méthode de récupération des streams populaires
- `TwitchApiClient` : appel API correspondant
- `HomeViewModel` : réception et transformation des données
- `HomeView.qml` : affichage de la section

## Fichiers concernés
- `src/ui/HomeView.qml`
- `src/ui/HomeViewModel.cpp`
- `src/ui/HomeViewModel.hpp`
- `src/api/twitch/TwitchService.cpp`
- `src/api/twitch/TwitchApiClient.cpp`

## Checklist de validation
- [ ] Identifier quelle section exacte est vide
- [ ] Vérifier que l'API est appelée au chargement
- [ ] Vérifier que l'API retourne des données (logs)
- [ ] Vérifier que les données sont transformées correctement
- [ ] Vérifier que le signal est émis vers le QML
- [ ] Vérifier que le QML reçoit les données
- [ ] La ligne affiche maintenant les streams
- [ ] Les thumbnails s'affichent
- [ ] Les informations (nom, viewers, jeu) s'affichent
- [ ] Clic sur un stream ouvre le player
- [ ] Pas de régression sur les autres sections
