# Plan 20 - Filtres par Catégorie/Jeu

## Contexte

La page d'accueil affiche actuellement des streams populaires, des catégories, et des streamers suivis. Il n'existe pas encore de moyen de filtrer ou naviguer par catégorie/jeu spécifique.

## Objectif

Permettre aux utilisateurs de :
1. Naviguer vers une catégorie/jeu depuis la Home
2. Voir les streams live d'une catégorie spécifique
3. Découvrir de nouveaux streamers par jeu

## Statut

**⚠️ PLACEHOLDER - Design à définir**

Ce plan est un placeholder. Le design et l'architecture nécessitent une session d'idéation dédiée car plusieurs questions restent ouvertes.

## Questions ouvertes

### Navigation
- [ ] Nouvelle vue dédiée `CategoryView.qml` ?
- [ ] Ou intégration dans la Home avec filtres dynamiques ?
- [ ] Ou page de recherche avancée ?

### UI
- [ ] Comment afficher les catégories ? Grille ? Liste ?
- [ ] Faut-il des sous-catégories ?
- [ ] Quelle hiérarchie de navigation ?

### Données
- [ ] Quels endpoints Twitch utiliser ?
- [ ] Faut-il du cache pour les catégories ?
- [ ] Pagination des résultats ?

## Pistes de réflexion

### Option A : CategoryView dédiée

```
Home
  └─→ Clic sur "Just Chatting"
        └─→ CategoryView (Just Chatting)
              ├─ Header : Image + Nom + Stats
              ├─ Filtres : Langue, Viewers, Tags
              └─ Liste des streams live
```

### Option B : Filtres dans la Home

```
Home
  ├─ [Catégorie: Toutes ▼] [Langue: Toutes ▼]
  │
  ├─ Streams filtrés
  └─ ...
```

### Option C : Page Explore

```
Explore
  ├─ Top Catégories (grille)
  ├─ Recherche par nom de jeu
  └─ Clic → Liste des streams
```

## API Twitch concernées

```
# Top catégories
GET /helix/games/top

# Streams par catégorie
GET /helix/streams?game_id=123

# Recherche de jeux
GET /helix/search/categories?query=...

# Détails d'un jeu
GET /helix/games?id=123
```

## Fichiers potentiellement concernés

- `src/ui/CategoryView.qml` (à créer)
- `src/ui/CategoryViewModel.cpp/hpp` (à créer)
- `src/ui/HomeView.qml` (si filtres intégrés)
- `src/ui/components/CategoryCard.qml` (existe déjà)
- `src/api/twitch/TwitchService.cpp/hpp`

## Prochaines étapes

1. **Session d'idéation** : Définir l'UX et le design
2. **Maquettes** : Créer des wireframes des options
3. **Choix d'architecture** : Décider de l'approche
4. **Mise à jour du plan** : Compléter les spécifications

## Checklist de validation

### À définir après idéation
- [ ] ...
- [ ] ...
- [ ] ...

---

**Note :** Ce plan sera complété lors d'une future session de planification.
