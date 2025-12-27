# Plan 14 - Bouton Follow/Unfollow

## Contexte

Actuellement, pour follow ou unfollow un streamer, l'utilisateur doit aller sur le site Twitch. Ajouter un bouton directement dans le player permettrait une expérience plus intégrée et fluide.

## Objectif

Ajouter un bouton Follow/Unfollow dans le PlayerView pour permettre à l'utilisateur de gérer ses follows sans quitter l'application.

## Spécifications

### UI - Bouton dans PlayerView

**Emplacement :** Dans la zone d'information du stream (près du nom du streamer) ou dans la PlayerControlBar.

**Design :**
```
État Unfollow (pas encore suivi) :
┌─────┐
│  ♡  │   Coeur vide, bordure visible
└─────┘

État Follow (déjà suivi) :
┌─────┐
│  ♥  │   Coeur plein, couleur Twitch/rouge
└─────┘
```

**Animation :** Transition douce entre les deux états (scale + fade).

### Comportement

1. **Clic sur coeur vide** → Appel API Follow → Animation → Coeur plein
2. **Clic sur coeur plein** → Appel API Unfollow → Animation → Coeur vide
3. **Pendant l'appel API** → État loading (opacity réduite ou spinner)
4. **En cas d'erreur** → Toast d'erreur, état inchangé

### API Twitch

**Endpoint Follow :**
```
POST https://api.twitch.tv/helix/channels/followed
Header: Authorization: Bearer <token>
Body: { "broadcaster_id": "123", "user_id": "456" }
```

**Endpoint Unfollow :**
```
DELETE https://api.twitch.tv/helix/channels/followed
Header: Authorization: Bearer <token>
Query: ?broadcaster_id=123&user_id=456
```

**Endpoint vérification (au chargement) :**
```
GET https://api.twitch.tv/helix/channels/followed
Query: ?user_id=456&broadcaster_id=123
```

### Gestion de l'état

- **Au chargement du PlayerView** : Vérifier si l'utilisateur follow déjà le streamer
- **Cache local** : Optionnel, pour éviter les appels répétés
- **Mode non connecté** : Bouton grisé avec tooltip "Connexion requise"

## Fichiers concernés

### À modifier
- `src/ui/PlayerView.qml` - Ajouter le bouton
- `src/api/twitch/TwitchService.cpp/hpp` - Méthodes follow/unfollow/isFollowing
- `src/api/twitch/TwitchApiClient.cpp/hpp` - Appels API

### À créer
- `src/ui/components/FollowButton.qml` - Composant réutilisable (optionnel)

## Checklist de validation

### UI
- [ ] Bouton visible dans le PlayerView
- [ ] Coeur vide quand non suivi
- [ ] Coeur plein quand suivi
- [ ] Animation de transition fluide
- [ ] État loading visible pendant l'appel API
- [ ] Bouton grisé si non connecté
- [ ] Tooltip "Connexion requise" si non connecté

### Fonctionnel
- [ ] Follow fonctionne (API appelée, état mis à jour)
- [ ] Unfollow fonctionne (API appelée, état mis à jour)
- [ ] État initial correct au chargement du player
- [ ] Gestion des erreurs avec toast

### API
- [ ] Token OAuth utilisé correctement
- [ ] Scopes nécessaires documentés
- [ ] Gestion du rate limiting

### Tests
- [ ] L'app compile sans erreur
- [ ] Pas de régression sur le player
- [ ] Fonctionne avec différents streamers
