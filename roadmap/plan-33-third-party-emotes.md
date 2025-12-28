# Plan 33 - Emotes BTTV/FFZ/7TV

## Contexte

Le chat Twitch natif n'affiche que les emotes officielles Twitch. Or, la communauté utilise massivement des emotes tierces provenant de BetterTTV (BTTV), FrankerFaceZ (FFZ) et 7TV. Sans ces emotes, le chat est incomplet et des messages apparaissent avec du texte là où il devrait y avoir des images.

## Objectif

Afficher les emotes BTTV, FFZ et 7TV dans le chat pour une expérience complète et fidèle à celle des extensions navigateur populaires.

## Specifications

### Comportement attendu

1. **Chargement des emotes**
   - Au démarrage du chat, récupérer les emotes globales de chaque service
   - Au chargement d'une chaîne, récupérer les emotes spécifiques à cette chaîne
   - Combiner les emotes globales et de chaîne dans un dictionnaire

2. **Affichage dans les messages**
   - Parser chaque message pour détecter les codes d'emotes (ex: `PepeHands`, `KEKW`)
   - Remplacer le texte par l'image correspondante
   - Conserver la taille cohérente avec les emotes Twitch natives

3. **Cache des emotes**
   - Stocker les images d'emotes localement pour éviter les requêtes répétées
   - Rafraîchir le cache périodiquement (ex: une fois par session)
   - Gérer l'espace disque (limite de taille ou nombre d'emotes)

4. **Priorité de résolution**
   - En cas de conflit de nom : 7TV > BTTV > FFZ (ou configurable)
   - Les emotes Twitch natives ont toujours la priorité absolue

### APIs publiques

- BTTV : `https://api.betterttv.net/`
- FFZ : `https://api.frankerfacez.com/`
- 7TV : `https://7tv.io/v3/`

## Fichiers concernes

- `src/chat/TwitchChatClient.cpp` - Intégration du parsing d'emotes
- Nouveau : `src/chat/ThirdPartyEmoteService.cpp/.hpp` - Service de récupération des emotes
- Nouveau : `src/chat/EmoteCache.cpp/.hpp` - Cache local des images

## Checklist de validation

### Chargement
- [ ] Les emotes globales BTTV sont récupérées au démarrage
- [ ] Les emotes globales FFZ sont récupérées au démarrage
- [ ] Les emotes globales 7TV sont récupérées au démarrage
- [ ] Les emotes de chaîne sont récupérées au changement de stream

### Affichage
- [ ] Les emotes BTTV s'affichent correctement dans les messages
- [ ] Les emotes FFZ s'affichent correctement dans les messages
- [ ] Les emotes 7TV s'affichent correctement dans les messages
- [ ] La taille des emotes est cohérente avec les emotes Twitch

### Cache
- [ ] Les emotes sont stockées localement
- [ ] Le cache est réutilisé entre les sessions
- [ ] La gestion de l'espace disque fonctionne

### Non-régression
- [ ] Les emotes Twitch natives fonctionnent toujours
- [ ] Le chat reste performant (pas de lag)
- [ ] L'application compile sans erreur
