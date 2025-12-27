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
| 9 (Vitesse lecture) | 5 (Cache/VOD) ✅ | Utilise le mode replay pour ajuster la vitesse |
| 12 (Préférences) | 11 (Sélecteur qualité) - partiel | Qualité par défaut nécessite le sélecteur |
| 13.7 (Sélecteur langue) | 12 (Préférences) | Le sélecteur sera dans le panel préférences |
| 15 (Historique VOD) | 5 (Cache/VOD) ✅ | Étend les métadonnées du cache existant |
| 16 (Audio only) | 11 (Qualité) - optionnel | Peut utiliser le même système de sélection |
| 18 (Thème) | 12 (Préférences) | Sélecteur de thème dans les préférences |
| 19.2-19.3 (Channels offline) | 6 (Search) ✅ | Étend la recherche existante |
| **24 (Refactoring)** | Aucune | Peut démarrer immédiatement, sous-tâches 24.1-24.3 CRITIQUES |

---

## Milestones suggérés

| Milestone | Plans inclus | Objectif |
|-----------|--------------|----------|
| **v0.7 - Core Features** | 7 ✅, 8 ✅, 11 | Chat + Qualité + Bugfix |
| **v0.8 - UX Polish** | 9 ✅, 10, 12 | Animations + Préférences |
| **v0.9 - i18n** | 13 | Internationalisation |
| **v0.10 - Sécurité** | 24.1-24.5 | Corrections critiques sécurité |
| **v1.0 - Release** | 21, 24.6-24.30 | Stabilité + Refactoring complet |
| **v1.x - Enhancements** | 14-20 | Nouvelles fonctionnalités |

> **Recommandation** : Exécuter le Plan 24 (sous-tâches 24.1-24.5) **IMMÉDIATEMENT** - problèmes de sécurité critiques.
> Exécuter le Plan 21 (Dette Technique) avant chaque release majeure.

---

## Suivi des tâches

| # | Tâche | Plan | Branche | Version | Statut |
|---|-------|------|---------|---------|--------|
| 1 | Bouton Volume | [plan-01](./plan-01-volume-button.md) | `feature/volume-button` | v0.1.0 | 🟢 Terminé |
| 2 | Fullscreen harmonisé | [plan-02](./plan-02-fullscreen.md) | `feature/fullscreen` | v0.2.0 | 🟢 Terminé |
| 3 | Largeur boutons | [plan-03](./plan-03-buttons-width.md) | `feature/buttons-width` | v0.3.0 | 🟢 Terminé |
| 4 | Layout barre de contrôle | [plan-04](./plan-04-controlbar-layout.md) | `feature/controlbar-layout` | v0.4.0 | 🟢 Terminé |
| 5 | Gestion Cache/VOD | [plan-05](./plan-05-cache-vod.md) | `feature/cache-vod` | v0.5.0 | 🟢 Terminé |
| 6 | Barre de recherche Twitch | [plan-06](./plan-06-search.md) | `feature/search` | v0.6.0 | 🟢 Terminé |
| 7 | Chat Twitch intégré | [plan-07](./plan-07-chat.md) | `feature/chat` | v0.7.0 | 🟢 Terminé |
| 8 | Fix ligne vide Home | [plan-08](./plan-08-home-empty-row.md) | `feature/fix-home-row` | v0.8.0 | 🟢 Terminé |
| 9 | Vitesse lecture intelligente | [plan-09](./plan-09-playback-speed.md) | `feature/playback-speed` | v0.9.0 | 🟢 Terminé |
| 10 | Animations & Transitions | [plan-10](./plan-10-animations.md) | `feature/animations` | v0.10.0 | 🟢 Terminé |
| 11 | Sélecteur Qualité Stream | [plan-11](./plan-11-quality-selector.md) | `feature/quality-selector` | - | 🔴 En attente |
| 12 | Panel Préférences | [plan-12](./plan-12-preferences.md) | `feature/preferences` | - | 🔴 En attente |
| 13 | Internationalisation (i18n) | [plan-13](./plan-13-internationalization.md) | `feature/i18n` | - | 🔴 En attente |
| 14 | Bouton Follow/Unfollow | [plan-14](./plan-14-follow-button.md) | `feature/follow-button` | - | 🔴 En attente |
| 15 | Historique VOD cachés | [plan-15](./plan-15-watch-history.md) | `feature/watch-history` | - | 🔴 En attente |
| 16 | Mode Audio uniquement | [plan-16](./plan-16-audio-only.md) | `feature/audio-only` | - | 🔴 En attente |
| 17 | Picture-in-Picture | [plan-17](./plan-17-picture-in-picture.md) | `feature/pip` | - | 🔴 En attente |
| 18 | Thème Clair/Sombre | [plan-18](./plan-18-theme-switcher.md) | `feature/theme` | - | 🔴 En attente |
| 19 | Twitch Tracker Stats | [plan-19](./plan-19-twitch-tracker.md) | `feature/twitch-tracker` | - | 🔴 En attente |
| 20 | Filtres Catégorie/Jeu | [plan-20](./plan-20-category-filters.md) | `feature/category-filters` | - | 🔴 En attente |
| **21** | **Dette Technique** | [plan-21](./plan-21-technical-debt.md) | `refactor/tech-debt` | - | 🔴 En attente |
| 22 | Page succès OAuth | [plan-22](./plan-22-oauth-success-page.md) | `fix/oauth-success-page` | - | 🔴 En attente |
| 23 | Auto-refresh Home | [plan-23](./plan-23-auto-refresh.md) | `feature/auto-refresh` | - | 🔴 En attente |
| **24** | **Refactoring Profond** | [plan-24](./plan-24-deep-refactoring.md) | `refactor/deep-analysis` | v0.13.0 (Sprint 3) | 🟡 En cours |
| **25** | **Audit UI/UX Design** | [plan-25](./plan-25-ui-design-audit.md) | `design/ui-audit` | - | 🔴 En attente |

### Plans spéciaux

| Type | Plan | Description |
|------|------|-------------|
| 🔄 Récurrent | Plan 21 | À exécuter après chaque milestone majeur |
| 📝 Placeholder | Plan 20 | Design à définir dans une future session |
| 🚨 Critique | Plan 24 | Refactoring technique issu d'analyse complète (25 sous-tâches) |
| 🎨 Design | Plan 25 | Audit UI/UX & Design System (20 sous-tâches) |

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
| 2025-12-27 | Tâche 6 terminée - Barre de recherche Twitch (channels live + cache local) |
| 2025-12-27 | Session d'idéation - Définition des nouvelles fonctionnalités (plans 14-20) |
| 2025-12-27 | Ajout plan 14 - Bouton Follow/Unfollow (PlayerView, coeur toggle) |
| 2025-12-27 | Ajout plan 15 - Historique VOD cachés (position reprise + date visionnage) |
| 2025-12-27 | Ajout plan 16 - Mode Audio uniquement (extension toggle HW/SW) |
| 2025-12-27 | Ajout plan 17 - Picture-in-Picture (fenêtre flottante macOS) |
| 2025-12-27 | Ajout plan 18 - Thème Clair/Sombre (auto système + override manuel) |
| 2025-12-27 | Ajout plan 19 - Twitch Tracker Stats (accès stats externes) |
| 2025-12-27 | Ajout plan 20 - Filtres Catégorie/Jeu (placeholder - design à définir) |
| 2025-12-27 | Ajout plan 21 - Dette Technique (audit, refactoring, performance) - plan récurrent |
| 2025-12-27 | Tâche 7 terminée - Chat Twitch intégré (lecture, envoi, emotes, badges) |
| 2025-12-27 | Ajout plan 22 - Page succès OAuth (amélioration UX authentification) |
| 2025-12-27 | Tâche 8 terminée - Suppression section redondante Home (5 sections au lieu de 6) |
| 2025-12-27 | Ajout plan 23 - Auto-refresh Home (30s streams/clips, retour premier plan, gestion erreur) |
| 2025-12-27 | Tâche 9 terminée - Vitesse lecture intelligente (auto-reset, protection live edge, mode replay) |
| 2025-12-27 | Ajout plan 24 - Refactoring Technique (25 sous-tâches : sécurité, QML refactoring, C++ refactoring, tests) |
| 2025-12-27 | Ajout plan 25 - Audit UI/UX Design (20 sous-tâches : palette, typographie, radius, composants, accessibilité) |
| 2025-12-27 | Tâche 10 terminée - Animations harmonisées, transitions navigation, architecture vues optimisée |
| 2025-12-27 | Plan 24 Sprint 1 terminé - Sécurité renforcée (Keychain, validation IRC, thread-safety) |
| 2025-12-27 | Plan 24 Sprint 2 terminé - Refactoring PlayerView (composants QML, logique recording C++, injection dépendances) |
| 2025-12-27 | Plan 24 Sprint 3 terminé - Refactoring PlayerControlBar (ControlButton, SeekBar, VolumeControl) + signature code |
