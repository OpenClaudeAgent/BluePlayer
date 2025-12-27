# Plan 07 - Chat Twitch intégré au Player

## Vue d'ensemble

Cette fonctionnalité permet d'afficher le chat Twitch directement dans le player, avec une gestion optimisée des performances.

---

## Objectifs

1. Afficher le chat Twitch dans le player
2. Activer/désactiver le chat via un bouton
3. Ne pas impacter les performances quand le chat est désactivé
4. Gérer le chat en mode VOD

---

## Contraintes de performance

**Problématique :**
Les streams populaires peuvent avoir des milliers de messages par minute dans le chat. Cela peut impacter significativement les performances de l'application.

**Stratégie :**
- **Connexion à la demande** : La connexion au chat ne s'établit que lorsque l'utilisateur clique sur le bouton
- **Déconnexion immédiate** : Quand le chat est fermé, la connexion est coupée
- **Pas de connexion par défaut** : Le chat est masqué et déconnecté au lancement du player
- **Throttling optionnel** : Limiter le nombre de messages affichés par seconde si nécessaire

---

## Sous-tâches

### 7.1 - Bouton d'affichage du chat (UI)

**Description :**
Ajouter un bouton dans la barre de contrôle du player pour afficher/masquer le chat.

**Éléments UI :**
- Icône de chat (bulle de dialogue)
- État actif/inactif visible
- Position : dans le groupe de droite de la PlayerControlBar

**Comportement :**
- Clic → Ouvre/ferme le panneau de chat
- Indicateur visuel quand le chat est actif

**Fichiers concernés :**
- `src/ui/components/PlayerControlBar.qml`
- `src/ui/PlayerView.qml`

---

### 7.2 - Panneau de chat (UI)

**Description :**
Créer le composant UI qui affiche les messages du chat.

**Éléments UI :**
- Panneau latéral (droite) ou overlay
- Liste scrollable des messages
- Affichage : username (avec couleur) + message
- Badges (sub, mod, vip, etc.) optionnels
- Emotes Twitch (optionnel, v2)

**Layout :**
```
┌─────────────────────────────────────┬──────────────┐
│                                     │   CHAT       │
│                                     │ ──────────── │
│           VIDEO                     │ user1: msg   │
│                                     │ user2: msg   │
│                                     │ user3: msg   │
│                                     │ ...          │
├─────────────────────────────────────┴──────────────┤
│                 CONTROL BAR                        │
└────────────────────────────────────────────────────┘
```

**Fichiers à créer :**
- `src/ui/components/ChatPanel.qml`
- `src/ui/components/ChatMessage.qml`

---

### 7.3 - Connexion au chat Twitch (Backend)

**Description :**
Implémenter la connexion au chat Twitch via IRC ou WebSocket.

**Protocole Twitch IRC :**
- Serveur : `irc.chat.twitch.tv:6667` (ou `wss://irc-ws.chat.twitch.tv:443` pour WebSocket)
- Authentification : OAuth token ou anonyme (lecture seule)
- Commandes : JOIN, PART, PRIVMSG

**Mode anonyme (recommandé pour lecture seule) :**
- Username : `justinfan<random>` (ex: `justinfan12345`)
- Pas besoin de token OAuth
- Lecture seule du chat

**Gestion de la connexion :**
- Connexion établie uniquement quand le chat est ouvert
- Déconnexion immédiate quand le chat est fermé
- Reconnexion automatique en cas de perte de connexion

**Fichiers à créer :**
- `src/chat/TwitchChatClient.hpp`
- `src/chat/TwitchChatClient.cpp`
- `src/chat/ChatMessage.hpp`

---

### 7.4 - Parsing des messages IRC

**Description :**
Parser les messages IRC Twitch pour extraire les informations utiles.

**Format des messages Twitch IRC :**
```
@badges=subscriber/12;color=#FF4500;display-name=Username;emotes=;... 
:username!username@username.tmi.twitch.tv PRIVMSG #channel :Message content
```

**Données à extraire :**
- Username (display-name)
- Couleur du username
- Message
- Badges (subscriber, moderator, vip, broadcaster)
- Emotes (positions dans le message)

**Fichiers concernés :**
- `src/chat/TwitchChatClient.cpp`
- `src/chat/ChatMessage.hpp`

---

### 7.5 - Gestion du mode VOD

**Description :**
Définir le comportement du chat quand on est en mode VOD (pas en live).

**Options possibles :**

1. **Pas de chat en VOD** (Recommandé pour v1)
   - Le bouton chat est grisé/masqué en mode VOD
   - Message explicatif : "Chat disponible uniquement en live"

2. **Chat replay (Complexe - v2)**
   - Utiliser l'API Twitch pour récupérer les messages du chat au moment de l'enregistrement
   - Synchroniser les messages avec la position de lecture
   - API : `https://api.twitch.tv/v5/videos/{video_id}/comments`

3. **Chat live de la chaîne (Alternative)**
   - Afficher le chat live actuel de la chaîne même en VOD
   - Permet d'interagir avec la communauté

**Recommandation :** Option 1 pour la v1, Option 2 pour une version future.

**Fichiers concernés :**
- `src/ui/PlayerView.qml`
- `src/chat/TwitchChatClient.cpp`

---

### 7.6 - Optimisations de performance

**Description :**
Implémenter des optimisations pour gérer les chats très actifs.

**Stratégies :**
- **Virtualisation de la liste** : Ne rendre que les messages visibles
- **Limite de messages** : Garder seulement les N derniers messages (ex: 500)
- **Throttling** : Limiter les mises à jour UI (ex: max 30 updates/sec)
- **Batch updates** : Grouper les messages reçus avant de mettre à jour l'UI

**Paramètres configurables :**
- Nombre max de messages à afficher
- Fréquence de mise à jour UI

**Fichiers concernés :**
- `src/ui/components/ChatPanel.qml`
- `src/chat/TwitchChatClient.cpp`

---

## Architecture proposée

```
┌─────────────────────────────────────────────────────────────┐
│                        PlayerView                            │
│  ┌─────────────────────────────────┬──────────────────────┐ │
│  │                                 │                      │ │
│  │         MpvQuickItem            │     ChatPanel        │ │
│  │         (Video)                 │     (Messages)       │ │
│  │                                 │                      │ │
│  └─────────────────────────────────┴──────────────────────┘ │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │              PlayerControlBar  [Chat Button]            │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    TwitchChatClient                          │
│  - Connexion IRC/WebSocket                                   │
│  - Parsing messages                                          │
│  - Gestion connexion/déconnexion                            │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Twitch IRC Server                         │
│                irc-ws.chat.twitch.tv:443                     │
└─────────────────────────────────────────────────────────────┘
```

---

## Priorité des sous-tâches

| Priorité | Sous-tâche | Dépendances |
|----------|------------|-------------|
| 1 | 7.3 - Connexion chat | Aucune |
| 2 | 7.4 - Parsing IRC | 7.3 |
| 3 | 7.2 - Panneau chat UI | Aucune |
| 4 | 7.1 - Bouton chat | 7.2 |
| 5 | 7.5 - Mode VOD | 7.1, 7.3 |
| 6 | 7.6 - Optimisations | 7.2, 7.3 |

---

## Checklist de validation

### 7.1 - Bouton chat
- [x] Bouton visible dans la PlayerControlBar (groupe de droite)
- [x] Icône bulle de dialogue
- [x] État actif/inactif clairement visible (couleur différente)
- [x] Clic ouvre/ferme le panneau de chat
- [x] Raccourci clavier (C) pour toggle
- [x] Bouton masqué en mode VOD

### 7.2 - Panneau chat UI
- [x] Panneau s'affiche à droite de la vidéo
- [x] Panneau redimensionnable (drag pour ajuster la largeur)
- [x] Largeur minimale et maximale respectées
- [x] Liste de messages scrollable avec virtualisation
- [x] Auto-scroll vers les nouveaux messages
- [x] Pause auto-scroll quand l'utilisateur remonte dans l'historique
- [x] Reprise auto-scroll quand l'utilisateur revient en bas
- [x] Header avec nom du channel et bouton fermer

### 7.3 - Connexion chat (IRC WebSocket)
- [x] Connexion via WebSocket (`wss://irc-ws.chat.twitch.tv:443`)
- [x] Mode anonyme avec username `justinfan<random>`
- [x] Connexion établie uniquement quand le chat est ouvert
- [x] Déconnexion immédiate quand le chat est fermé
- [x] Reconnexion automatique en cas de perte (max 3 tentatives)
- [x] Indicateur de statut connexion (connecté/déconnecté/erreur)
- [x] Pas de connexion au lancement du player

### 7.4 - Parsing messages IRC
- [x] Username extrait depuis `display-name`
- [x] Couleur du username appliquée depuis `color`
- [x] Message correctement affiché
- [x] Badges parsés depuis `badges` (subscriber, moderator, vip, broadcaster)
- [x] Badges affichés avec icônes réelles (images Twitch)
- [x] Emotes parsées depuis `emotes` (positions dans le message)
- [x] Emotes affichées comme images inline
- [x] Caractères spéciaux et unicode gérés

### 7.5 - Mode VOD
- [x] Bouton chat masqué en mode VOD
- [x] Pas de tentative de connexion en VOD

### 7.6 - Performance
- [x] Limite de 500 messages en mémoire (FIFO)
- [x] Batch updates : grouper les messages (max 30 updates/sec)
- [x] Virtualisation de la liste (seuls les messages visibles sont rendus)
- [x] Cache des emotes téléchargées
- [x] Cache des badges téléchargés
- [x] Pas de lag avec un chat actif (1000+ msg/min)
- [x] Mémoire stable (pas de fuite)
- [x] Pas d'impact sur la lecture vidéo

### Tests généraux
- [x] Fonctionne avec différentes chaînes (petit et gros chat)
- [x] Gestion des erreurs réseau (affichage message d'erreur)
- [x] L'application compile sans erreur
- [x] Les tests passent

---

## Bonus (ajouté lors de l'implémentation)

- **Emotes Twitch inline** : Les emotes sont affichées comme images dans le texte du message
- **Badges avec icônes réelles** : Les badges (mod, vip, sub, broadcaster) utilisent les vraies icônes Twitch
- **Panneau redimensionnable** : L'utilisateur peut ajuster la largeur du panneau chat en glissant le bord
- **Cache des assets** : Les emotes et badges sont téléchargés une fois et mis en cache
- **Envoi de messages** : L'utilisateur authentifié peut envoyer des messages dans le chat
- **Écho local** : Les messages envoyés s'affichent immédiatement sans attendre le serveur
- **Système de logging fichier** : Les logs sont enregistrés dans un fichier pour faciliter le debug
