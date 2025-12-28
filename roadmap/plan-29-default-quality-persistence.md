# Plan 29 - Persistance et Intégration de la Qualité par Défaut

## Contexte

Le Plan 12 (Panel Préférences) a implémenté l'UI d'un dropdown "Qualité par défaut" dans les préférences. Cependant, cette fonctionnalité est actuellement purement visuelle :

1. **La valeur n'est pas persistée** : Quand l'application redémarre, la qualité revient toujours à "Auto"
2. **La valeur n'est pas utilisée** : Quand on ouvre un stream, la préférence de l'utilisateur n'est pas prise en compte
3. **Pas de synchronisation** : Si l'utilisateur change la qualité dans le player, les préférences ne sont pas mises à jour

Ce plan complète le cycle de la fonctionnalité "qualité par défaut" en ajoutant la persistance et l'intégration avec le lecteur.

## Objectifs

1. **Persistance** : Sauvegarder le choix de qualité par défaut de l'utilisateur entre les sessions
2. **Intégration au lecteur** : Appliquer automatiquement cette qualité quand un stream ou VOD s'ouvre
3. **Fallback intelligent** : Si la qualité demandée n'est pas disponible, choisir la plus proche

## Comportement attendu

### Scénario 1 : Sauvegarde de la préférence

1. L'utilisateur ouvre les Préférences
2. Il sélectionne "720p60" dans le dropdown "Qualité par défaut"
3. La valeur est immédiatement sauvegardée
4. L'utilisateur ferme l'application
5. Au prochain lancement, le dropdown affiche "720p60" (et non "Auto")

### Scénario 2 : Application de la qualité au démarrage d'un stream

1. L'utilisateur a configuré "1080p" comme qualité par défaut
2. Il lance un stream live
3. Le stream démarre automatiquement en 1080p (si disponible)
4. Le sélecteur de qualité dans le player affiche "1080p" comme sélectionné

### Scénario 3 : Fallback intelligent

1. L'utilisateur a configuré "1080p60" comme qualité par défaut
2. Il lance un stream qui n'offre que : 1080p, 720p60, 720p, 480p
3. Le système sélectionne automatiquement "1080p" (la plus proche inférieure)
4. Aucun message d'erreur n'est affiché (comportement silencieux)

### Scénario 4 : Option "Auto"

1. L'utilisateur a configuré "Auto" comme qualité par défaut
2. Il lance un stream
3. Le comportement actuel est conservé (meilleure qualité disponible)

### Scénario 5 : VOD cachée

1. L'utilisateur a configuré "720p" comme qualité par défaut
2. Il lit une VOD depuis le cache local
3. La qualité d'enregistrement originale est utilisée (ignorant la préférence)
4. Le sélecteur de qualité n'est pas disponible pour les VOD cachées

### Scénario 6 : Changement de qualité dans le player (optionnel)

1. L'utilisateur a "Auto" configuré par défaut
2. Pendant la lecture d'un stream, il change manuellement la qualité à "720p"
3. Cette modification N'affecte PAS la préférence sauvegardée
4. Le prochain stream utilisera toujours "Auto"

> **Note** : La synchronisation bidirectionnelle (player → préférences) est explicitement hors scope pour garder le comportement prévisible.

## Hiérarchie de priorité des qualités

Pour le fallback intelligent, utiliser cette hiérarchie (de la meilleure à la moins bonne) :

```
1080p60 > 1080p > 720p60 > 720p > 480p > 360p > 160p > audio_only
```

**Règle de fallback** : Si la qualité demandée n'est pas disponible, sélectionner la première qualité disponible INFÉRIEURE dans la hiérarchie. Si aucune qualité inférieure n'est disponible, sélectionner la première supérieure.

## Fichiers concernés

- `src/ui/PreferencesView.qml` - UI du dropdown (déjà fait)
- `src/api/twitch/TwitchService.hpp/.cpp` - Application de la qualité
- `src/core/Config.hpp/.cpp` - Persistance de la préférence

## Dépendances

### Ce plan dépend de

| Plan | Titre | Statut |
|------|-------|--------|
| Plan 11 | Sélecteur Qualité Stream | ✅ Terminé |
| Plan 12 | Panel Préférences | ✅ Terminé |

### Ce plan est requis par

- Aucun plan identifié pour l'instant

## Checklist de validation

### Persistance
- [ ] La qualité par défaut sélectionnée persiste après fermeture de l'application
- [ ] Au redémarrage, le dropdown affiche la valeur sauvegardée
- [ ] La sauvegarde est immédiate (pas de bouton "Enregistrer")

### Intégration lecteur - Stream live
- [ ] Ouvrir un stream live applique automatiquement la qualité par défaut
- [ ] Le sélecteur de qualité dans le player reflète la qualité appliquée
- [ ] Le changement est silencieux (pas de popup, pas de notification)

### Intégration lecteur - VOD Twitch
- [ ] Ouvrir une VOD Twitch applique automatiquement la qualité par défaut
- [ ] Même comportement que pour les streams live

### Fallback intelligent
- [ ] Si qualité demandée non disponible, la plus proche inférieure est sélectionnée
- [ ] Si aucune qualité inférieure, la plus proche supérieure est sélectionnée
- [ ] Aucun message d'erreur affiché lors du fallback

### Option Auto
- [ ] Avec "Auto" configuré, le comportement actuel est préservé
- [ ] "Auto" sélectionne la meilleure qualité disponible

### VOD cachée
- [ ] Les VOD cachées utilisent leur qualité d'enregistrement originale
- [ ] La préférence de qualité par défaut est ignorée pour les VOD cachées

### Cas limites
- [ ] Stream avec une seule qualité disponible : cette qualité est utilisée
- [ ] Qualité "audio_only" demandée : fonctionne si disponible
- [ ] Premier lancement (aucune préférence) : comportement "Auto"

### Non-régression
- [ ] Le sélecteur de qualité dans le player fonctionne toujours
- [ ] Changer la qualité manuellement dans le player fonctionne toujours
- [ ] Les autres préférences (cache) ne sont pas affectées

---

*Plan créé le 2025-12-28 - Complète la fonctionnalité UI du Plan 12*
