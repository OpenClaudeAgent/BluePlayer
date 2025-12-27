# Plan 15 - Historique de visionnage VOD

## Contexte

Le système de cache VOD (plan-05) permet de sauvegarder des VOD localement. Actuellement, il n'y a pas de suivi de la progression de visionnage. L'utilisateur ne peut pas reprendre un VOD là où il s'était arrêté.

## Objectif

Enrichir les métadonnées des VOD cachés pour :
1. Sauvegarder la **position de reprise** (timestamp)
2. Enregistrer la **date de dernier visionnage**
3. Permettre de **filtrer par date de visionnage** dans le CacheManagerView

## Spécifications

### Métadonnées à ajouter

```cpp
// Extension de VodMetadata ou WatchHistory
struct WatchProgress {
    QString vodId;
    double lastPosition;      // Position en secondes
    double totalDuration;     // Durée totale
    QDateTime lastWatchedAt;  // Date de dernier visionnage
    bool completed;           // true si vu à > 90%
};
```

### Comportement

#### Sauvegarde automatique
- **Quand ?** Toutes les 30 secondes + à la fermeture du player
- **Quoi ?** Position actuelle, date/heure, durée totale
- **Où ?** Fichier JSON dans le dossier de cache ou base SQLite existante

#### Reprise automatique
- **Au chargement d'un VOD caché** : Vérifier s'il existe une progression
- **Si oui** : Proposer "Reprendre à XX:XX" ou "Recommencer"
- **Si non** : Lecture depuis le début

#### UI - Dialog de reprise

```
┌─────────────────────────────────────────┐
│                                         │
│   Reprendre la lecture ?                │
│                                         │
│   Vous étiez à 45:23 / 2:15:00          │
│                                         │
│   [Reprendre]     [Recommencer]         │
│                                         │
└─────────────────────────────────────────┘
```

### Filtre dans CacheManagerView

Ajouter un filtre/tri parmi les options existantes :
- **Tri par date de visionnage** (plus récent en premier)
- **Indicateur visuel** : Barre de progression sur les cards

```
┌─────────────────────────────────────┐
│  [Thumbnail]                        │
│  Titre du VOD                       │
│  Streamer - 2h15                    │
│  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░  45%     │  ← Progression
│  Vu il y a 2 jours                  │
└─────────────────────────────────────┘
```

## Dépendances

| Ce plan | Dépend de | Raison |
|---------|-----------|--------|
| 15 | Plan 5 (Cache VOD) ✅ | Utilise le système de cache existant |

## Fichiers concernés

### À modifier
- `src/core/VodMetadata.hpp` - Ajouter champs progression
- `src/core/WatchHistory.cpp/hpp` - Logique de sauvegarde/lecture
- `src/core/CacheManager.cpp/hpp` - Intégration avec le cache
- `src/ui/CacheManagerView.qml` - Affichage progression + filtre
- `src/ui/CacheManagerViewModel.cpp/hpp` - Exposer les données
- `src/ui/PlayerView.qml` - Dialog de reprise

### À créer
- `src/ui/components/ResumeDialog.qml` - Dialog de reprise (optionnel)

## Checklist de validation

### Sauvegarde
- [ ] Position sauvegardée toutes les 30 secondes
- [ ] Position sauvegardée à la fermeture du player
- [ ] Date de visionnage enregistrée
- [ ] Durée totale enregistrée
- [ ] Marqué "complété" si vu à > 90%

### Reprise
- [ ] Dialog de reprise affiché si progression existante
- [ ] Bouton "Reprendre" fonctionne (seek à la position)
- [ ] Bouton "Recommencer" fonctionne (lecture depuis 0)
- [ ] Pas de dialog si pas de progression

### UI CacheManagerView
- [ ] Barre de progression visible sur les cards
- [ ] Pourcentage affiché
- [ ] "Vu il y a X jours" affiché
- [ ] Tri par date de visionnage disponible
- [ ] Tri fonctionne correctement

### Persistance
- [ ] Données persistées après redémarrage
- [ ] Pas de corruption des données existantes
- [ ] Migration propre si structure change

### Tests
- [ ] L'app compile sans erreur
- [ ] Pas de régression sur le cache existant
- [ ] Performance acceptable (pas de lag à la sauvegarde)
