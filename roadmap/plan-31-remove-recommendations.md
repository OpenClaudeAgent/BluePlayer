# Plan 31 - Supprimer la section "Recommandation"

## Contexte

La section "Recommandation" dans la HomeView ne fonctionne plus et devait être supprimée. Elle est toujours présente dans l'interface et affiche un contenu cassé ou vide.

## Objectif

Supprimer définitivement cette section de la HomeView pour nettoyer l'interface utilisateur.

## Spécifications

### Comportement attendu

1. La section "Recommandation" n'apparaît plus dans la HomeView
2. Les autres sections restent intactes et fonctionnelles
3. L'espacement entre les sections restantes est correct

### Impact

- Nettoyage de l'interface : moins de sections = interface plus claire
- Simplification du code : suppression du code mort

## Fichiers concernés

- `src/ui/HomeView.qml` - Suppression de la section QML
- `src/ui/HomeViewModel.cpp` - Suppression de la logique associée (si présente)
- `src/ui/HomeViewModel.hpp` - Suppression des propriétés associées (si présentes)

## Checklist de validation

### Suppression
- [ ] La section "Recommandation" n'est plus visible dans la Home
- [ ] Le code QML de la section est supprimé
- [ ] La logique C++ associée est supprimée (si applicable)

### Non-régression
- [ ] Les autres sections de la Home fonctionnent normalement
- [ ] L'espacement entre les sections est correct
- [ ] L'application compile sans erreur
- [ ] Pas de warning lié à des propriétés manquantes
