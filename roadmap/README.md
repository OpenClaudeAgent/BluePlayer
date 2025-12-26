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

## Suivi des tâches

| # | Tâche | Plan | Branche | Statut |
|---|-------|------|---------|--------|
| 1 | Bouton Volume | [plan-01](./plan-01-volume-button.md) | `feature/volume-button` | 🟢 Terminé |
| 2 | Vitesse lecture auto-reset | [plan-02](./plan-02-playback-speed.md) | `feature/playback-speed` | 🔴 En attente |
| 3 | Fullscreen harmonisé | [plan-03](./plan-03-fullscreen.md) | `feature/fullscreen` | 🔴 En attente |
| 4 | Largeur boutons | [plan-04](./plan-04-buttons-width.md) | `feature/buttons-width` | 🔴 En attente |
| 5 | Layout barre de contrôle | [plan-05](./plan-05-controlbar-layout.md) | `feature/controlbar-layout` | 🔴 En attente |
| 6 | Gestion Cache/VOD | [plan-06](./plan-06-cache-vod.md) | `feature/cache-vod` | 🔴 En attente |
| 7 | Barre de recherche Twitch | [plan-07](./plan-07-search.md) | `feature/search` | 🔴 En attente |
| 8 | Chat Twitch intégré | [plan-08](./plan-08-chat.md) | `feature/chat` | 🔴 En attente |
| 9 | Fix ligne vide Home | [plan-09](./plan-09-home-empty-row.md) | `feature/fix-home-row` | 🔴 En attente |
| 10 | Panel Préférences | [plan-10](./plan-10-preferences.md) | `feature/preferences` | 🔴 En attente |

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
| 2024-XX-XX | Création de la roadmap, tâches 1-9 définies |
| 2024-12-26 | Ajout tâche 10 - Panel Préférences |
| 2024-12-26 | Tâche 1 terminée - Bouton Volume (slider vertical + hover) |
