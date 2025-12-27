# Plan 08 - Fix ligne vide Home Page

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
- [x] Identifier quelle section exacte est vide
- [x] Vérifier que l'API est appelée au chargement
- [x] Vérifier que l'API retourne des données (logs)
- [x] Vérifier que les données sont transformées correctement
- [x] Vérifier que le signal est émis vers le QML
- [x] Vérifier que le QML reçoit les données
- [x] Plus de ligne vide sur la Home
- [x] Pas de régression sur les autres sections

---

## Solution adoptée

La section "En direct maintenant" était redondante avec "Recommandé pour vous" (même source de données). 
**Décision** : Suppression de la section au lieu de la remplir avec des données dupliquées.

**Sections finales (5 au lieu de 6) :**
1. Vos streamers suivis
2. Recommandé pour vous
3. Parcourir (catégories)
4. Clips populaires
5. Recommandations par catégorie
