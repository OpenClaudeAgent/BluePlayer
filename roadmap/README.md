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
| 16 (Audio only) | 11 (Qualité) ✅ | Base technique implémentée, reste le placeholder visuel |
| 18 (Thème) | 12 (Préférences) | Sélecteur de thème dans les préférences |
| 19.2-19.3 (Channels offline) | 6 (Search) ✅ | Étend la recherche existante |
| **24 (Refactoring)** | Aucune | Peut démarrer immédiatement, sous-tâches 24.1-24.3 CRITIQUES |
| **28 (Navigation)** | Aucune | Recommandé avant Plan 12 (Préférences) pour architecture propre |
| 29 (Qualité défaut) | 11 (Qualité) ✅, 12 (Préférences) ✅ | Persistance et intégration de l'UI créée en Plan 12 |

---

## Versions livrées

| Version | Plan | Contenu |
|---------|------|---------|
| **v0.7.0** | Plan 7 | Chat Twitch intégré |
| **v0.8.0** | Plan 8 | Fix ligne vide Home |
| **v0.9.0** | Plan 9 | Vitesse lecture intelligente |
| **v0.10.0** | Plan 10 | Animations & Transitions |
| **v0.11.0** | Plan 24 Sprint 1 | Sécurité (Keychain, validation IRC, thread-safety) |
| **v0.12.0** | Plan 24 Sprint 2 | Refactoring PlayerView |
| **v0.13.0** | Plan 24 Sprint 3 | Refactoring PlayerControlBar |
| **v0.14.0** | Plan 24 Sprint 4 | BaseCard & Cards refactorées |
| **v0.15.0** | Plan 24 Sprint 5 | TwitchService helper |
| **v0.16.0** | Plan 24 Sprint 6 | downloadThumbnail() asynchrone |

### Prochaines versions prévues

| Version | Plan | Objectif |
|---------|------|----------|
| **v0.17.0** | Plan 24 Sprint 7 | Tests & CI/CD |
| **v1.0** | Plans 11-13 | Qualité, Préférences, i18n |
| **v1.x** | Plans 14-20 | Nouvelles fonctionnalités |

> **Note** : Exécuter le Plan 21 (Dette Technique) avant chaque release majeure.

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
| 11 | Sélecteur Qualité Stream | [plan-11](./plan-11-quality-selector.md) | `feature/quality-selector` | v0.18.0 | 🟢 Terminé |
| 12 | Panel Préférences | [plan-12](./plan-12-preferences.md) | `feature/preferences` | v0.19.0 | 🟢 Terminé |
| 13 | Internationalisation (i18n) | [plan-13](./plan-13-internationalization.md) | `feature/i18n` | v0.22.0 | 🟢 Terminé |
| 14 | Bouton Follow/Unfollow | [plan-14](./plan-14-follow-button.md) | `feature/follow-button` | - | ❌ Annulé (API Twitch non disponible) |
| 15 | Historique VOD cachés | [plan-15](./plan-15-watch-history.md) | `feature/watch-history` | v0.23.0 | 🟢 Terminé |
| 16 | Mode Audio uniquement | [plan-16](./plan-16-audio-only.md) | `feature/audio-only` | v0.24.0 | 🟢 Terminé |
| 17 | Picture-in-Picture | [plan-17](./plan-17-picture-in-picture.md) | `feature/pip` | v0.25.0 | 🟢 Terminé |
| 18 | Thème Clair/Sombre | [plan-18](./plan-18-theme-switcher.md) | `feature/theme` | v0.26.0 | 🟢 Terminé |
| 19 | Twitch Tracker Stats | [plan-19](./plan-19-twitch-tracker.md) | `feature/twitch-tracker` | - | 🔴 En attente |
| 20 | Filtres Catégorie/Jeu | [plan-20](./plan-20-category-filters.md) | `feature/category-filters` | - | 🔴 En attente |
| **21** | **Dette Technique** | [plan-21](./plan-21-technical-debt.md) | `refactor/tech-debt` | - | 🔴 En attente |
| 22 | Page succès OAuth | [plan-22](./plan-22-oauth-success-page.md) | `fix/oauth-success-page` | - | 🔴 En attente |
| 23 | Auto-refresh Home | [plan-23](./plan-23-auto-refresh.md) | `feature/auto-refresh` | - | 🔴 En attente |
| **24** | **Refactoring Profond** | [plan-24](./plan-24-deep-refactoring.md) | `refactor/deep-analysis` | v0.17.0 (Sprint 7) | 🟢 Terminé |
| **25** | **Audit UI/UX Design** | [plan-25](./plan-25-ui-design-audit.md) | `design/ui-audit` | - | 🔴 En attente |
| 26 | Icône Application BluePlayer | [plan-26](./plan-26-app-icon.md) | `feature/app-icon` | - | 🔴 En attente |
| 27 | Suppression MpvFboItem (Legacy) | [plan-27](./plan-27-remove-mpvfboitem.md) | `refactor/remove-mpvfboitem` | - | 🔴 En attente |
| 28 | Strategie Navigation Globale | [plan-28](./plan-28-navigation-strategy.md) | `feature/navigation-strategy` | v0.20.0 | 🟢 Terminé |
| 29 | Persistance Qualité par Défaut | [plan-29](./plan-29-default-quality-persistence.md) | `feature/default-quality` | v0.21.0 | 🟢 Terminé |
| 30 | Harmonisation des Toasts | [plan-30](./plan-30-toast-harmonization.md) | `feature/toast-harmonization` | - | 🔴 En attente |

### Plans spéciaux

| Type | Plan | Description |
|------|------|-------------|
| 🔄 Récurrent | Plan 21 | **TEMPLATE** - Méthodologie d'audit dette technique à réexécuter périodiquement |
| 📝 Placeholder | Plan 20 | Design à définir dans une future session |
| 🚨 Critique | Plan 24 | **INSTANCE** - Audit Q4 2025 (27 décembre) basé sur le template Plan 21 |
| 🎨 Design | Plan 25 | Audit UI/UX & Design System (20 sous-tâches) |
| 🧹 Cleanup | Plan 27 | Suppression code legacy (remplace tâche 24.20) |
| 🏗️ Architecture | Plan 28 | Stratégie navigation globale (TopBar, composants réutilisables) |

> **Note Plan 21 vs Plan 24** :
> - **Plan 21** est un template récurrent définissant la méthodologie d'audit qualité
> - **Plan 24** est une instance concrète de ce template, exécutée le 27 décembre 2025
> - Les prochains audits créeront de nouveaux plans (ex: Plan 28, Plan 35...) basés sur Plan 21

### Légende des statuts

- 🔴 En attente
- 🟡 En cours
- 🟢 Terminé
- ⏸️ En pause
- ❌ Annulé

---

## Historique des implémentations

| Date | Implémentation |
|------|----------------|
| 2025-12-26 | Tâche 1 terminée - Bouton Volume (slider vertical + hover) |
| 2025-12-26 | Tâche 2 terminée - Fullscreen harmonisé (comportement unifié + curseur auto-caché) |
| 2025-12-26 | Tâche 3 terminée - Largeur boutons harmonisée (64px) + style uniforme |
| 2025-12-26 | Tâche 4 terminée - Layout barre de contrôle réorganisé + timer intelligent |
| 2025-12-26 | Tâche 5 terminée - Gestion Cache/VOD (enregistrement auto, thumbnails, mode replay, nettoyage LRU) |
| 2025-12-27 | Tâche 6 terminée - Barre de recherche Twitch (channels live + cache local) |
| 2025-12-27 | Tâche 7 terminée - Chat Twitch intégré (lecture, envoi, emotes, badges) |
| 2025-12-27 | Tâche 8 terminée - Suppression section redondante Home (5 sections au lieu de 6) |
| 2025-12-27 | Tâche 9 terminée - Vitesse lecture intelligente (auto-reset, protection live edge, mode replay) |
| 2025-12-27 | Tâche 10 terminée - Animations harmonisées, transitions navigation, architecture vues optimisée |
| 2025-12-27 | Plan 24 Sprint 1 terminé - Sécurité renforcée (Keychain, validation IRC, thread-safety) |
| 2025-12-27 | Plan 24 Sprint 2 terminé - Refactoring PlayerView (composants QML, logique recording C++, injection dépendances) |
| 2025-12-27 | Plan 24 Sprint 3 terminé - Refactoring PlayerControlBar (ControlButton, SeekBar, VolumeControl) + signature code |
| 2025-12-28 | Plan 24 Sprint 4 terminé - BaseCard créé, 5 cards refactorées (-239 lignes, -26%) |
| 2025-12-28 | Plan 24 Sprint 5 terminé - TwitchService helper ensureTokenAndExecute() (-29 lignes, 9 duplications) |
| 2025-12-28 | Plan 24 Sprint 6 terminé - downloadThumbnail() asynchrone (thread UI non bloqué) |
| 2025-12-28 | Plan 24 Sprint 7 terminé - +32 tests (unitaires + intégration), CI/CD reporté |
| 2025-12-28 | **Plan 24 TERMINÉ** - Refactoring Profond complet (7 sprints, v0.11.0 → v0.17.0) |
| 2025-12-28 | Tâche 11 terminée - Sélecteur qualité stream (bouton HD/SD, popup, qualité VOD cachée) |
| 2025-12-28 | Tâche 12 terminée - Panel Préférences modernisé (design, dropdown qualité, cache, BlueDropdown) |
| 2025-12-28 | Tâche 28 terminée - Navigation globale (CircleButton, PanelHeader, boutons cachés dans panels) |
| 2025-12-28 | Tâche 29 terminée - Persistance qualité par défaut (QSettings, TwitchService, fallback intelligent) |
| 2025-12-28 | Tâche 13 terminée - Internationalisation (i18n) avec changement de langue à chaud |
| 2025-12-28 | Tâche 14 annulée - API Twitch ne permet pas de follow/unfollow programmatiquement |
| 2025-12-28 | Tâche 15 terminée - Historique VOD (reprise auto, sauvegarde 30s, tri "Dernier vu") |
| 2025-12-28 | Tâche 16 terminée - Mode Audio (placeholder visuel, labels normalisés, toast redesigné) |
| 2025-12-28 | Tâche 17 terminée - Picture-in-Picture (fenêtre flottante, contrôles hover, ratio 16:9) |
| 2025-12-28 | Tâche 18 terminée - Thème Clair/Sombre (détection système, sélecteur, migration composants) |

---

## Historique de la roadmap

| Date | Évolution |
|------|-----------|
| 2025-12-27 | Création de la roadmap, tâches 1-9 définies |
| 2025-12-26 | Ajout tâche 10 - Panel Préférences |
| 2025-12-26 | Réorganisation : vitesse lecture (ex plan-02) devient plan-09, dépend de plan-05 (Cache/VOD) |
| 2025-12-27 | Ajout colonne Version au tableau de suivi + tags git v0.1.0 à v0.5.0 |
| 2025-12-27 | Correction numérotation : plans 9-12 synchronisés avec fichiers réels |
| 2025-12-27 | Ajout tâche 13 - Internationalisation (i18n) avec 7 sous-tâches |
| 2025-12-27 | Session d'idéation - Définition plans 14-20 (Follow, Historique, Audio, PiP, Thème, Stats, Filtres) |
| 2025-12-27 | Ajout plan 21 - Dette Technique (audit, refactoring, performance) - plan récurrent |
| 2025-12-27 | Ajout plan 22 - Page succès OAuth (amélioration UX authentification) |
| 2025-12-27 | Ajout plan 23 - Auto-refresh Home (30s streams/clips, retour premier plan, gestion erreur) |
| 2025-12-27 | Ajout plan 24 - Refactoring Technique (25 sous-tâches : sécurité, QML, C++, tests) |
| 2025-12-27 | Ajout plan 25 - Audit UI/UX Design (20 sous-tâches : palette, typographie, composants) |
| 2025-12-28 | Ajout plan 26 - Icône Application BluePlayer (intégration icône "Blue Drop Play" validée) |
| 2025-12-28 | Ajout plan 27 - Suppression MpvFboItem (code legacy mort, remplace tâche 24.20) |
| 2025-12-28 | Ajout plan 28 - Stratégie Navigation Globale (TopBar, CircleButton, refactoring headers) |
| 2025-12-28 | Ajout plan 29 - Persistance Qualité par Défaut (persistance, intégration TwitchService, fallback) |
| 2025-12-28 | Ajout plan 30 - Harmonisation des Toasts (feedback visuel pour tous les contrôles player) |
