# Plan 41 - Gestion et Nettoyage des Logs

## Contexte

L'application BluePlayer utilise un systeme de logging via `FileLogger` qui cree un nouveau fichier a chaque demarrage dans `~/Library/Caches/BluePlayer/logs/`. Apres seulement 2-3 jours d'utilisation, le dossier contient deja **130 fichiers de logs** pour un total de **4.3 MB**.

Le probleme est double :
1. **Aucun mecanisme de nettoyage** : Les fichiers s'accumulent indefiniment
2. **Logs excessifs** : Beaucoup de logs DEBUG verbeux, redondants ou temporaires qui polluent les fichiers

### Analyse d'un fichier de log reel (20 min de session)

| Categorie | Lignes | % du total |
|-----------|--------|------------|
| **Total** | 2528 | 100% |
| Chat RAW messages (TwitchChatClient) | 1498 | **59%** |
| TypeError repete (ChatMessage.qml:85) | 893 | **35%** |
| Double prefixe DEBUG | 29 | 1% |
| JSON debugging temporaire | 12 | 0.5% |
| **Logs utiles estimes** | ~96 | **~4%** |

**Conclusion : 96% des logs sont du bruit inutile !**

## Objectif

Mettre en place une gestion saine des logs avec :
1. Un mecanisme de rotation automatique qui supprime les vieux fichiers
2. Un audit du code pour nettoyer les logs inutiles ou excessifs
3. Une strategie de logging claire avec des niveaux appropries
4. Des logs pertinents sur les flux critiques (navigation, fonctionnalites importantes)

> **Note** : Le bug ChatMessage.qml:85 est traite separement dans le [Plan 42](./plan-42-fix-chatmessage-null.md)

## Comportement attendu

### 41.1 - Rotation automatique des logs

**Au demarrage de l'application :**
- Le `FileLogger` verifie le contenu du dossier `logs/`
- Les fichiers de logs de plus de **7 jours** sont automatiquement supprimes
- Si plus de **20 fichiers** existent, les plus anciens sont supprimes pour garder seulement 20
- Un message de log indique combien de fichiers ont ete nettoyes

**Configuration suggeree :**
| Parametre | Valeur par defaut |
|-----------|-------------------|
| Retention maximale | 7 jours |
| Nombre max de fichiers | 20 |

### 41.2 - Audit et nettoyage des logs applicatifs

#### Logs a SUPPRIMER (bruit pur)

| Source | Probleme | Action |
|--------|----------|--------|
| `TwitchChatClient.cpp:221` | Log RAW de CHAQUE message IRC | **Supprimer** |
| Fichiers QML | JSON debug (sessionId, hypothesisId) | **Supprimer** |
| Double prefixe | `[DEBUG] [DEBUG HomeView]` | **Corriger format** |

> **Note** : Le bug `ChatMessage.qml:85` (TypeError repete 893x) est traite dans le [Plan 42](./plan-42-fix-chatmessage-null.md)

#### Logs a REDUIRE (trop verbeux)

| Fichier | Appels actuels | Probleme |
|---------|----------------|----------|
| `TwitchApiClient.cpp` | 149 | Logs excessifs |
| `TwitchService.cpp` | 101 | Logs excessifs |
| `HomeView.qml` | 38 | Logs de chaque changement d'etat |
| `PlayerView.qml` | 42 | Logs de chaque action |
| `TwitchAuthManager.cpp` | 34 | Logs de tokens (sensible) |

#### Logs sensibles a SUPPRIMER

```
[DEBUG] [DEBUG] OAuth Client-ID: kpt3f7w9eh9y4kg53g0zbe0grqhabm  <- Client-ID complet !
[DEBUG] Loaded access token: ecisk8zl... (length: 30)             <- Token partiel
[DEBUG] Loaded refresh token, length: 50                          <- Info sensible
```

#### Logs redondants a FUSIONNER

```
[DEBUG] [PlayerView] Downloading thumbnail from: https://...
[INFO] Downloading thumbnail from: https://...                    <- Doublon !

[INFO] Cache cleanup service started, interval: 300000ms
[INFO] Cache cleanup service started                              <- Doublon !
```

#### Logs a CONSERVER

- Erreurs et warnings (sauf le TypeError repete)
- Initialisation de l'application
- Connexion/deconnexion utilisateur
- Demarrage/arret lecture video
- Changements de qualite
- Operations de cache importantes

### 41.3 - Configuration des niveaux de log

**Niveaux par defaut en production :**
| Categorie | Niveau |
|-----------|--------|
| Media | INFO |
| Twitch | INFO |
| UI | WARNING |
| Core | INFO |
| Network | WARNING |

Les niveaux DEBUG ne devraient etre actives que via variables d'environnement pour le debugging.

### 41.4 - Strategie de logging par niveaux

#### Definition des niveaux

| Niveau | Usage | Exemples |
|--------|-------|----------|
| **DEBUG** | Developpement uniquement, details techniques | Valeurs de variables, etats internes, traces |
| **INFO** | Evenements normaux importants | Demarrage, connexion, actions utilisateur |
| **WARNING** | Situations anormales non bloquantes | Timeout, retry, fallback utilise |
| **ERROR** | Erreurs recuperables | Echec API, fichier non trouve |
| **CRITICAL** | Erreurs fatales | Crash imminent, corruption donnees |

#### Regles d'utilisation

1. **DEBUG** : Jamais en production sauf activation explicite via env var
2. **INFO** : Evenements "jalons" du cycle de vie de l'app
3. **WARNING** : Quelque chose d'inattendu mais l'app continue
4. **ERROR** : Quelque chose a echoue, action utilisateur impactee
5. **CRITICAL** : L'app ne peut plus fonctionner correctement

### 41.5 - Logs a AJOUTER sur les flux critiques

Certains flux importants manquent de logs. Il faut ajouter des logs **INFO** sur :

#### Navigation

| Evenement | Log attendu |
|-----------|-------------|
| Changement de vue | `[INFO] [Navigation] View changed: home -> player` |
| Ouverture panel | `[INFO] [Navigation] Panel opened: preferences` |
| Fermeture panel | `[INFO] [Navigation] Panel closed: preferences` |
| Retour arriere | `[INFO] [Navigation] Back navigation to: home` |

#### Lecture video

| Evenement | Log attendu |
|-----------|-------------|
| Demarrage stream | `[INFO] [Player] Stream started: squeezie (live)` |
| Demarrage VOD | `[INFO] [Player] VOD started: video_id (cached)` |
| Arret lecture | `[INFO] [Player] Playback stopped` |
| Changement qualite | `[INFO] [Player] Quality changed: 1080p -> 720p` |
| Erreur lecture | `[ERROR] [Player] Playback error: <message>` |

#### Authentification

| Evenement | Log attendu |
|-----------|-------------|
| Login reussi | `[INFO] [Auth] User authenticated: <username>` |
| Logout | `[INFO] [Auth] User logged out` |
| Token refresh | `[INFO] [Auth] Token refreshed successfully` |
| Echec auth | `[ERROR] [Auth] Authentication failed: <reason>` |

#### Cache

| Evenement | Log attendu |
|-----------|-------------|
| Enregistrement demarre | `[INFO] [Cache] Recording started: squeezie` |
| Enregistrement termine | `[INFO] [Cache] Recording stopped: squeezie (duration: 15min)` |
| VOD supprimee | `[INFO] [Cache] VOD deleted: video_id` |
| Nettoyage cache | `[INFO] [Cache] Cleanup: removed 3 old files, freed 150MB` |

#### Chat

| Evenement | Log attendu |
|-----------|-------------|
| Connexion chat | `[INFO] [Chat] Connected to channel: squeezie` |
| Deconnexion chat | `[INFO] [Chat] Disconnected from channel: squeezie` |
| Erreur chat | `[ERROR] [Chat] Connection error: <message>` |

> **Important** : Ne PAS logger le contenu des messages, seulement les evenements de connexion/deconnexion.

### 41.6 - Format de log standardise

Tous les logs doivent suivre ce format :

```
[NIVEAU] [Categorie] Message concis
```

Exemples :
- `[INFO] [Player] Stream started: squeezie (live)`
- `[ERROR] [Auth] Token refresh failed: 401 Unauthorized`
- `[WARNING] [Cache] Disk space low, cleanup recommended`

**Categories valides :**
- `Navigation` - Changements de vue et panels
- `Player` - Lecture video/audio
- `Auth` - Authentification et tokens
- `Cache` - Gestion du cache VOD
- `Chat` - Client IRC Twitch
- `API` - Appels API Twitch
- `Network` - Requetes HTTP
- `Core` - Initialisation et cycle de vie app

## Fichiers concernes

### Rotation automatique
- `src/core/FileLogger.hpp`
- `src/core/FileLogger.cpp`

### Logs C++ a auditer

| Fichier | Appels | Priorite |
|---------|--------|----------|
| `src/chat/TwitchChatClient.cpp` | 16 (qDebug) | **CRITIQUE** |
| `src/api/twitch/TwitchApiClient.cpp` | 149 (Logger) | Haute |
| `src/api/twitch/TwitchService.cpp` | 101 (Logger) | Haute |
| `src/api/twitch/TwitchAuthManager.cpp` | 34 (Logger) | Haute |
| `src/core/CacheManager.cpp` | 47 (LOG_*) | Moyenne |
| `src/ui/HomeViewModel.cpp` | 17 (Logger) | Moyenne |
| `src/core/SecureStorage.cpp` | 14 (Logger) | Moyenne |
| `src/core/StateMachineLiveReplay.cpp` | 13 (LOG_*) | Moyenne |
| `src/core/WatchHistory.cpp` | 9 (Logger) | Basse |
| `src/core/network/CurlHttpClient.cpp` | 9 (Logger) | Basse |
| `src/media/MpvQuickItem.cpp` | 7 (qDebug) | Basse |

### Logs QML a auditer

| Fichier | Appels console.* | Priorite |
|---------|------------------|----------|
| `src/ui/PlayerView.qml` | 42 | **CRITIQUE** |
| `src/ui/HomeView.qml` | 38 | Haute |
| `src/ui/main.qml` | 17 | Haute |
| `src/ui/LoginView.qml` | 10 | Moyenne |
| `src/ui/PreferencesView.qml` | 6 | Basse |
| `src/ui/components/StreamCard.qml` | 2 | Basse |
| `src/ui/components/HorizontalRowSection.qml` | 2 | Basse |
| `src/ui/components/ThemeProvider.qml` | 2 | Basse |
| Autres components | 4 | Basse |

### Configuration
- `src/core/Logger.cpp`
- `src/core/Config.cpp`

## Checklist de validation

### Rotation automatique
- [ ] Les fichiers de plus de 7 jours sont supprimes au demarrage
- [ ] Le nombre de fichiers est limite a 20 maximum
- [ ] Un message de log indique le nombre de fichiers nettoyes
- [ ] La suppression est silencieuse si rien a nettoyer

### Audit des logs C++
- [ ] TwitchChatClient : log RAW (ligne 221) supprime
- [ ] TwitchAuthManager : tokens/Client-ID ne sont plus logges
- [ ] Logs redondants fusionnes ou supprimes
- [ ] Pas de double prefixe `[DEBUG] [DEBUG ...]`

### Audit des logs QML
- [ ] Logs JSON de debugging temporaires supprimes (sessionId, hypothesisId)
- [ ] PlayerView.qml : logs reduits a l'essentiel
- [ ] HomeView.qml : logs reduits a l'essentiel
- [ ] main.qml : logs reduits a l'essentiel

### Strategie de logging
- [ ] Format standardise `[NIVEAU] [Categorie] Message` applique partout
- [ ] Logs INFO ajoutes pour la navigation (changements de vue, panels)
- [ ] Logs INFO ajoutes pour le player (demarrage, arret, changement qualite)
- [ ] Logs INFO ajoutes pour l'authentification (login, logout, refresh)
- [ ] Logs INFO ajoutes pour le cache (enregistrement, suppression, cleanup)
- [ ] Logs INFO ajoutes pour le chat (connexion, deconnexion)
- [ ] Niveaux DEBUG actives uniquement via variable d'environnement

### Objectif global
- [ ] **Reduction de 90% minimum du volume de logs** (de 2500 lignes a ~250 pour 20 min)
- [ ] Logs restants sont utiles et actionnables
- [ ] Les flux critiques sont traces avec des logs INFO pertinents

### Tests
- [ ] Test unitaire : suppression fichiers > 7 jours
- [ ] Test unitaire : limite de 20 fichiers
- [ ] Test unitaire : pas de suppression si < 20 fichiers recents
- [ ] Application demarre et logge correctement apres les changements
