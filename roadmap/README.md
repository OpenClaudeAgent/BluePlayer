# BluePlayer - Roadmap

## Instructions globales

### Méthodologie de travail

1. **Planification** : Discuter avec l'utilisateur, définir les besoins, créer un fichier plan
2. **Création du plan** : Le plan est créé une seule fois et devient **immutable**
3. **Implémentation** : Créer une branche dédiée, implémenter, valider avec l'utilisateur
4. **Validation** : Parcourir la checklist avec l'utilisateur avant de merger
5. **Merge** : Merger sur main uniquement après validation

### Règles importantes

- **Les fichiers de plan sont immutables** : Une fois créé, un plan ne doit plus être modifié
- **Le statut est géré ici** : Seul ce fichier README est mis à jour pour suivre l'avancement
- **Validation obligatoire** : Chaque tâche doit être validée par l'utilisateur avant merge
- **Une branche par tâche** : Chaque tâche a sa propre branche Git

### Cycle de vie d'une tâche

```
[Planification] → [Plan créé] → [Branche créée] → [Implémentation] → [Validation] → [Merge]
     📝              📄              🌿               💻              ✅           🔀
```

---

## Template de plan

Chaque fichier de plan doit suivre cette structure :

```markdown
# Plan XX - [Titre de la tâche]

## Contexte
[Description du problème ou de la fonctionnalité]

## Objectif
[Ce qu'on veut accomplir]

## Spécifications
[Détails techniques, comportement attendu]

## Fichiers concernés
- `path/to/file1`
- `path/to/file2`

## Checklist de validation
- [ ] Point 1
- [ ] Point 2
- [ ] Point 3
```

Pour les tâches complexes avec sous-tâches, ajouter :

```markdown
## Sous-tâches
- X.1 - [Sous-tâche 1]
- X.2 - [Sous-tâche 2]

## Priorité des sous-tâches
| Priorité | Sous-tâche | Dépendances |
|----------|------------|-------------|
| 1 | X.1 | Aucune |
| 2 | X.2 | X.1 |
```

---

## Dépendances entre tâches

| Tâche | Dépend de | Notes |
|-------|-----------|-------|
| 9 (Vitesse lecture) | 5 (Cache/VOD) | La vitesse variable n'a de sens que pour les VOD |
| 12 (Préférences) | 11 (Qualité selector) | Pour implémenter "qualité par défaut" |

### Ordre d'implémentation recommandé

Pour les features liées à la qualité :
```
Plan 11 (Sélection qualité) → Plan 12 (Préférences avec qualité par défaut)
```

---

## Suivi des tâches

| # | Tâche | Plan | Branche | Statut |
|---|-------|------|---------|--------|
| 1 | Bouton Volume | [plan-01](./plan-01-volume-button.md) | `feature/volume-button` | 🟢 Terminé |
| 2 | Fullscreen harmonisé | [plan-02](./plan-02-fullscreen.md) | `feature/fullscreen` | 🔴 En attente |
| 3 | Largeur boutons | [plan-03](./plan-03-buttons-width.md) | `feature/buttons-width` | 🔴 En attente |
| 4 | Layout barre de contrôle | [plan-04](./plan-04-controlbar-layout.md) | `feature/controlbar-layout` | 🔴 En attente |
| 5 | Gestion Cache/VOD | [plan-05](./plan-05-cache-vod.md) | `feature/cache-vod` | 🔴 En attente |
| 6 | Barre de recherche Twitch | [plan-06](./plan-06-search.md) | `feature/search` | 🔴 En attente |
| 7 | Chat Twitch intégré | [plan-07](./plan-07-chat.md) | `feature/chat` | 🔴 En attente |
| 8 | Fix ligne vide Home | [plan-08](./plan-08-home-empty-row.md) | `feature/fix-home-row` | 🔴 En attente |
| 9 | Vitesse lecture intelligente | [plan-09](./plan-09-playback-speed.md) | `feature/playback-speed` | 🔴 En attente |
| 10 | Stratégie animations globales | [plan-10](./plan-10-animations.md) | `feature/animations` | 🔴 En attente |
| 11 | Sélection qualité stream | [plan-11](./plan-11-quality-selector.md) | `feature/quality-selector` | 🔴 En attente |
| 12 | Panel Préférences | [plan-12](./plan-12-preferences.md) | `feature/preferences` | 🔴 En attente |

### Légende des statuts

- 🔴 En attente
- 🟡 En cours
- 🟢 Terminé
- ⏸️ En pause
- ❌ Annulé

---

## Historique des changements

| Date | Changement |
|------|------------|
| 2025-12-XX | Création de la roadmap, tâches 1-9 définies |
| 2025-12-26 | Ajout tâche 10 - Panel Préférences |
| 2025-12-26 | Tâche 1 terminée - Bouton Volume (slider vertical + hover) |
| 2025-12-26 | Réorganisation : vitesse lecture déplacée, dépend de plan-05 (Cache/VOD) |
| 2025-12-26 | Ajout plan-10 (Animations) et plan-11 (Qualité stream) |
| 2025-12-26 | Renumérotation finale : 9=Vitesse, 10=Animations, 11=Qualité, 12=Préférences |
| 2025-12-26 | Dépendance ajoutée : Plan 12 (Préférences) dépend de Plan 11 (Qualité) |
