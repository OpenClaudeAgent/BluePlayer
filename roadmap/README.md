# BluePlayer - Roadmap

## Instructions globales

### Méthodologie de travail

1. **Planification** : Discuter avec l'utilisateur, définir les besoins, créer un fichier plan
2. **Création du plan** : Le plan est créé une seule fois et devient **immutable**
3. **Implémentation** : Créer une branche dédiée, implémenter, valider avec l'utilisateur
4. **Validation** : Parcourir la checklist avec l'utilisateur avant de merger
5. **Merge** : Merger sur main uniquement après validation

### Règles importantes

- **Les fichiers de plan sont immutables** : Une fois créé, un plan ne doit plus être modifié (sauf les checkboxes de validation qui peuvent être cochées)
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

| Tâche | Dépend de | Raison |
|-------|-----------|--------|
| 9 (Vitesse lecture) | 5 (Cache/VOD) | Utilise le mode replay pour ajuster la vitesse |
| 12 (Préférences) | 11 (Sélecteur qualité) - partiel | Qualité par défaut nécessite le sélecteur |
| 13.7 (Sélecteur langue) | 12 (Préférences) | Le sélecteur sera dans le panel préférences |

---

## Suivi des tâches

| # | Tâche | Plan | Branche | Version | Statut |
|---|-------|------|---------|---------|--------|
| 1 | Bouton Volume | [plan-01](./plan-01-volume-button.md) | `feature/volume-button` | v0.1.0 | 🟢 Terminé |
| 2 | Fullscreen harmonisé | [plan-02](./plan-02-fullscreen.md) | `feature/fullscreen` | v0.2.0 | 🟢 Terminé |
| 3 | Largeur boutons | [plan-03](./plan-03-buttons-width.md) | `feature/buttons-width` | v0.3.0 | 🟢 Terminé |
| 4 | Layout barre de contrôle | [plan-04](./plan-04-controlbar-layout.md) | `feature/controlbar-layout` | v0.4.0 | 🟢 Terminé |
| 5 | Gestion Cache/VOD | [plan-05](./plan-05-cache-vod.md) | `feature/cache-vod` | v0.5.0 | 🟢 Terminé |
| 6 | Barre de recherche Twitch | [plan-06](./plan-06-search.md) | `feature/search` | - | 🔴 En attente |
| 7 | Chat Twitch intégré | [plan-07](./plan-07-chat.md) | `feature/chat` | - | 🔴 En attente |
| 8 | Fix ligne vide Home | [plan-08](./plan-08-home-empty-row.md) | `feature/fix-home-row` | - | 🔴 En attente |
| 9 | Vitesse lecture intelligente | [plan-09](./plan-09-playback-speed.md) | `feature/playback-speed` | - | 🔴 En attente |
| 10 | Animations & Transitions | [plan-10](./plan-10-animations.md) | `feature/animations` | - | 🔴 En attente |
| 11 | Sélecteur Qualité Stream | [plan-11](./plan-11-quality-selector.md) | `feature/quality-selector` | - | 🔴 En attente |
| 12 | Panel Préférences | [plan-12](./plan-12-preferences.md) | `feature/preferences` | - | 🔴 En attente |
| 13 | Internationalisation (i18n) | [plan-13](./plan-13-internationalization.md) | `feature/i18n` | - | 🔴 En attente |

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
| 2025-12-26 | Réorganisation roadmap : vitesse lecture (ex plan-02) devient plan-09, dépend de plan-05 (Cache/VOD) |
| 2025-12-26 | Tâche 2 terminée - Fullscreen harmonisé (comportement unifié + curseur auto-caché) |
| 2025-12-26 | Tâche 3 terminée - Largeur boutons harmonisée (64px) + style uniforme |
| 2025-12-26 | Tâche 4 terminée - Layout barre de contrôle réorganisé + timer intelligent |
| 2025-12-26 | Tâche 5 terminée - Gestion Cache/VOD (enregistrement auto, thumbnails, mode replay, nettoyage LRU) |
| 2025-12-27 | Ajout colonne Version au tableau de suivi + tags git v0.1.0 à v0.5.0 |
| 2025-12-27 | Correction numérotation : plans 9-12 synchronisés avec fichiers réels |
| 2025-12-27 | Ajout tâche 13 - Internationalisation (i18n) avec 7 sous-tâches |
