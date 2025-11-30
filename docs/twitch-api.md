# Notes API Twitch

## Ressources clés
- Documentation officielle : https://dev.twitch.tv/docs/api/
- Authentification OAuth 2.0 (PKCE conseillé pour application desktop).
- Endpoints cibles :
  - `GET /streams?user_login={channel}` -> état live + IDs.
  - `GET /videos?user_id={id}` -> VODs et manifestes.
  - `GET /channels/{id}` -> métadonnées supplémentaires.

## Authentification
- Créer une application sur https://dev.twitch.tv/console/apps pour obtenir `Client ID`.
- Gérer le flux Authorization Code + PKCE pour éviter d’exposer un secret.
- Stocker `access_token` + `refresh_token` chiffrés dans le profil utilisateur.
- Renouveler automatiquement avant expiration (expires_in ~4h).

## Manifestes / Lecture
- Les URLs HLS/DASH sont fournies indirectement :
  1. Appel API -> obtention `stream id`.
  2. Requête vers l’endpoint "usher" (non documenté) avec les bons paramètres (`sig`, `token`).
- Les paramètres nécessaires (`sig`, `token`, `url`) sont retournés par l’API `Playback Access Token` :
  - `GET https://gql.twitch.tv/gql` avec la requête GraphQL `PlaybackAccessToken`.
  - Headers : `Client-ID`, `Authorization: Bearer <token>`.

## Contraintes & Quotas
- 800 requêtes / minute / utilisateur par défaut -> implémenter un rate limiter.
- Réessayer avec backoff exponentiel sur les erreurs 429.

## Actions à réaliser
1. Créer `TwitchAuthManager` (QtNetwork + PKCE).
2. Implémenter `TwitchApiClient` pour les endpoints REST + GQL.
3. Ajouter tests unitaires simulant les réponses JSON (module `tests/api`).

