# Plan - Gestion du Cache et des VOD

## Vue d'ensemble

Cette fonctionnalité permet de gérer les vidéos enregistrées dans le cache local, de les rejouer, et de contrôler l'espace disque utilisé par l'application.

---

## Objectifs

1. Accéder aux replays depuis la page d'accueil
2. Visualiser et gérer les VOD en cache
3. Rejouer les VOD enregistrées
4. Contrôler la taille du cache
5. Nettoyer le cache (manuel et automatique)

---

## Sous-tâches

### 6.1 - Bouton d'accès aux Replays (Home Page)

**Description :**
Ajouter un bouton/section sur la page d'accueil permettant d'accéder à l'écran de gestion des replays.

**Éléments :**
- Bouton ou carte "Mes Replays" / "Cache local"
- Indication du nombre de vidéos en cache
- Indication de l'espace utilisé (ex: "2.3 GB utilisés")

**Fichiers concernés :**
- `src/ui/HomeView.qml`
- `src/ui/HomeViewModel.cpp`

---

### 6.2 - Écran de gestion des VOD (Cache Manager UI)

**Description :**
Nouvel écran dédié à la visualisation et gestion des VOD en cache.

**Éléments UI :**
- Liste des VOD avec aperçu (thumbnail si disponible)
- Métadonnées affichées pour chaque VOD :
  - Nom du streamer
  - Titre du stream
  - Date d'enregistrement
  - Durée
  - Taille du fichier
- Mode sélection (checkboxes) pour suppression multiple
- Bouton "Supprimer la sélection"
- Bouton "Tout supprimer"
- Tri et filtrage (par date, taille, streamer)

**Fichiers à créer :**
- `src/ui/CacheManagerView.qml`
- `src/ui/CacheManagerViewModel.hpp`
- `src/ui/CacheManagerViewModel.cpp`

---

### 6.3 - Stockage des métadonnées VOD

**Description :**
Définir un format de stockage pour les métadonnées des VOD enregistrées.

**Structure de données suggérée :**
```json
{
  "id": "uuid-unique",
  "streamerLogin": "streamer123",
  "streamerName": "Streamer Name",
  "streamTitle": "Titre du stream",
  "recordedAt": "2024-01-15T14:30:00Z",
  "duration": 3600,
  "fileSize": 1073741824,
  "filePath": "/path/to/cache/file.ts",
  "thumbnailPath": "/path/to/thumbnail.jpg",
  "gameCategory": "Just Chatting"
}
```

**Fichiers concernés :**
- `src/core/CacheManager.hpp`
- `src/core/CacheManager.cpp`
- Nouveau : `src/core/VodMetadata.hpp`

---

### 6.4 - Lecture des VOD en cache

**Description :**
Permettre de lancer la lecture d'une VOD depuis l'écran de gestion du cache.

**Comportement :**
- Clic sur une VOD → ouvre le PlayerView en mode VOD
- Mode VOD : pas de mode live, seekbar libre
- Affichage des métadonnées (streamer, titre) dans le player
- Possibilité de reprendre là où on s'était arrêté (optionnel, v2)

**Fichiers concernés :**
- `src/ui/PlayerView.qml`
- `src/ui/CacheManagerView.qml`
- `src/media/MpvQuickItem.cpp`

---

### 6.5 - Configuration de la taille maximale du cache

**Description :**
Permettre à l'utilisateur de définir la taille maximale du cache.

**Éléments UI :**
- Slider ou input pour définir la taille max (ex: 1GB - 50GB)
- Affichage de l'espace utilisé vs espace max
- Barre de progression visuelle
- Sauvegarde dans les préférences

**Fichiers concernés :**
- `src/ui/CacheManagerView.qml` ou `src/ui/PreferencesView.qml`
- `src/core/CacheManager.cpp`
- `src/core/Config.cpp`

---

### 6.6 - Stratégie de nettoyage automatique du cache

**Description :**
Implémenter un service de nettoyage automatique qui tourne en arrière-plan pendant toute la durée de vie de l'application.

**Critères importants :**
- **Service applicatif** : Ce n'est PAS un service OS, mais un service interne à l'application
- **Exécution continue** : Doit tourner en arrière-plan tant que l'application est lancée
- **Vérification périodique** : Doit régulièrement vérifier :
  - La taille actuelle du cache consommé
  - L'espace maximum alloué par l'utilisateur
- **Nettoyage proactif** : Déclenche le nettoyage automatiquement quand le seuil est atteint

**Implémentation suggérée :**
- QTimer avec intervalle configurable (ex: toutes les 5 minutes)
- Ou surveillance via QFileSystemWatcher sur le dossier de cache
- Thread dédié ou exécution dans le thread principal avec tâches légères

**Stratégies de nettoyage possibles :**
1. **LRU (Least Recently Used)** : Supprimer les VOD les moins récemment visionnées
2. **FIFO (First In First Out)** : Supprimer les plus anciennes
3. **Par taille** : Supprimer les plus volumineuses d'abord
4. **Hybride** : Combinaison de critères

**Stratégie recommandée : LRU**
- Garder un timestamp "lastPlayedAt" pour chaque VOD
- Quand le cache dépasse le seuil (ex: 90% de la limite), supprimer les VOD non visionnées depuis le plus longtemps
- Continuer à supprimer jusqu'à atteindre un seuil confortable (ex: 80% de la limite)
- Notifier l'utilisateur quand une suppression automatique a lieu

**Fichiers concernés :**
- `src/core/CacheManager.cpp`
- `src/core/CacheManager.hpp`
- `src/core/VodMetadata.hpp`
- `src/core/Application.cpp` (initialisation du service)

---

### 6.7 - Suppression manuelle des VOD

**Description :**
Permettre la suppression manuelle des VOD.

**Modes de suppression :**
1. **Individuelle** : Swipe-to-delete ou bouton sur chaque item
2. **Multiple** : Mode sélection avec checkboxes
3. **Totale** : Bouton "Vider le cache"

**Confirmation :**
- Dialog de confirmation avant suppression
- Indication de l'espace qui sera libéré

**Fichiers concernés :**
- `src/ui/CacheManagerView.qml`
- `src/core/CacheManager.cpp`

---

### 6.8 - Enregistrement automatique pendant le visionnage live

**Description :**
S'assurer que le cache DVR existant est correctement sauvegardé avec ses métadonnées pour pouvoir être relu plus tard.

**Comportement :**
- Pendant un live, le buffer DVR est déjà en cache (fonctionnalité existante)
- À la fermeture du player ou après X minutes, sauvegarder les métadonnées
- Option : demander à l'utilisateur s'il veut garder l'enregistrement

**Fichiers concernés :**
- `src/media/MpvQuickItem.cpp`
- `src/core/CacheManager.cpp`
- `src/ui/PlayerView.qml`

---

## Architecture proposée

```
┌─────────────────────────────────────────────────────────────┐
│                        HomeView                              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ Live Streams│  │ Categories  │  │ 📁 Mes Replays     │  │
│  └─────────────┘  └─────────────┘  │    12 vidéos       │  │
│                                     │    2.3 GB          │  │
│                                     └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    CacheManagerView                          │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ Cache: 2.3 GB / 10 GB  [================    ]  Settings ││
│  └─────────────────────────────────────────────────────────┘│
│                                                              │
│  ☐ StreamerA - "Just Chatting"     2h30  │  1.2 GB  │ 🗑️   │
│  ☐ StreamerB - "Gaming Session"    45min │  380 MB  │ 🗑️   │
│  ☐ StreamerC - "Coding Live"       3h    │  1.5 GB  │ 🗑️   │
│                                                              │
│  [Supprimer la sélection]              [Tout supprimer]     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      PlayerView (VOD)                        │
│  ┌─────────────────────────────────────────────────────────┐│
│  │                                                          ││
│  │                    [VIDEO CONTENT]                       ││
│  │                                                          ││
│  └─────────────────────────────────────────────────────────┘│
│  StreamerA - "Just Chatting" - 15 Jan 2024                  │
│  [▶] [════════════════════════════] 01:23:45 / 02:30:00    │
└─────────────────────────────────────────────────────────────┘
```

---

## Priorité des sous-tâches

| Priorité | Sous-tâche | Dépendances |
|----------|------------|-------------|
| 1 | 6.3 - Stockage métadonnées | Aucune |
| 2 | 6.8 - Enregistrement auto | 6.3 |
| 3 | 6.2 - UI Cache Manager | 6.3 |
| 4 | 6.1 - Bouton Home | 6.2 |
| 5 | 6.4 - Lecture VOD | 6.2, 6.3 |
| 6 | 6.7 - Suppression manuelle | 6.2 |
| 7 | 6.5 - Config taille max | 6.2 |
| 8 | 6.6 - Nettoyage auto | 6.5, 6.3 |

---

## Checklist de validation

### 6.1 - Bouton d'accès aux Replays
- [ ] Bouton/carte visible sur la Home Page
- [ ] Affiche le nombre de VOD en cache
- [ ] Affiche l'espace utilisé
- [ ] Clic ouvre l'écran de gestion du cache

### 6.2 - Écran de gestion des VOD
- [ ] Liste des VOD affichée correctement
- [ ] Thumbnails affichés (si disponibles)
- [ ] Métadonnées visibles (streamer, titre, date, durée, taille)
- [ ] Mode sélection (checkboxes) fonctionne
- [ ] Tri par date fonctionne
- [ ] Tri par taille fonctionne
- [ ] Filtrage par streamer fonctionne
- [ ] UI responsive et fluide

### 6.3 - Stockage des métadonnées
- [ ] Structure de données définie
- [ ] Métadonnées sauvegardées correctement
- [ ] Métadonnées chargées au démarrage
- [ ] Pas de perte de données entre les sessions

### 6.4 - Lecture des VOD
- [ ] Clic sur une VOD lance le player
- [ ] Player en mode VOD (pas live)
- [ ] Seekbar fonctionne sur toute la durée
- [ ] Métadonnées affichées dans le player
- [ ] Retour à l'écran de gestion après fermeture

### 6.5 - Configuration taille max
- [ ] UI pour définir la taille max
- [ ] Valeur sauvegardée dans les préférences
- [ ] Valeur chargée au démarrage
- [ ] Barre de progression espace utilisé/max

### 6.6 - Nettoyage automatique
- [ ] Service de nettoyage initialisé au démarrage de l'application
- [ ] Service tourne en arrière-plan pendant toute la durée de vie de l'app
- [ ] Vérification périodique de la taille du cache (intervalle configurable)
- [ ] Comparaison avec l'espace max alloué par l'utilisateur
- [ ] Détection quand le cache dépasse le seuil (ex: 90%)
- [ ] Suppression des VOD selon la stratégie LRU
- [ ] Nettoyage jusqu'à un seuil confortable (ex: 80%)
- [ ] Notification à l'utilisateur lors d'une suppression automatique
- [ ] Espace libéré correctement
- [ ] Pas d'impact sur les performances de l'application

### 6.7 - Suppression manuelle
- [ ] Suppression individuelle fonctionne
- [ ] Suppression multiple fonctionne
- [ ] "Tout supprimer" fonctionne
- [ ] Dialog de confirmation affiché
- [ ] Espace disque libéré correctement
- [ ] Liste mise à jour après suppression

### 6.8 - Enregistrement automatique
- [ ] Métadonnées sauvegardées à la fermeture du player
- [ ] Buffer DVR conservé après fermeture
- [ ] VOD apparaît dans la liste du cache manager

### Tests généraux
- [ ] Pas de fuite mémoire
- [ ] Performances acceptables avec beaucoup de VOD
- [ ] Gestion des erreurs (fichier corrompu, disque plein, etc.)
- [ ] L'application compile sans erreur
- [ ] Les tests unitaires passent
