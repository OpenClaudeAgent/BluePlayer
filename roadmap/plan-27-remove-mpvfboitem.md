# Plan 27 - Suppression MpvFboItem (Code Legacy)

## Contexte

Suite à l'analyse du Sprint 6 du Plan 24 (Refactoring C++ Media), il a été constaté que `MpvFboItem` est du **code 100% mort** :

- Non enregistré pour QML (seul `MpvQuickItem` l'est dans `main.mm`)
- Jamais utilisé dans aucun fichier `.qml`
- Implémentation obsolète et incomplète par rapport à `MpvQuickItem`

### Historique

| Aspect | MpvFboItem (legacy) | MpvQuickItem (actuel) |
|--------|--------------------|-----------------------|
| Base Qt | `QQuickFramebufferObject` | `QQuickPaintedItem` |
| Rendu | OpenGL FBO | Software (QImage + QPainter) |
| Fonctionnalités | Basiques (~10 props) | Complètes (~20+ props) |
| Live mode | Non | Oui |
| Recording | Non | Oui |
| Playback rate | Non | Oui |
| Thread safety | Non (bools simples) | Oui (std::atomic) |
| Utilisé | Non | Oui |

Le plan 24.20 prévoyait de créer une classe de base commune (`MpvPlayerBase`), mais cette approche a été abandonnée car :
1. `MpvFboItem` n'est pas utilisé
2. Créer une abstraction pour du code mort est contre-productif
3. La suppression est la solution la plus simple et la plus propre

## Objectif

Supprimer le code legacy `MpvFboItem` qui n'est jamais utilisé, afin de :
- Réduire la dette technique
- Clarifier le codebase (une seule implémentation mpv)
- Éliminer la confusion potentielle pour les futurs développeurs

## Comportement attendu

Après suppression :
- Le projet compile sans erreur
- Tous les tests passent
- La lecture vidéo fonctionne normalement (via `MpvQuickItem`)
- Aucune régression fonctionnelle

## Fichiers concernés

### À supprimer
- `src/media/MpvFboItem.hpp` (99 lignes)
- `src/media/MpvFboItem.cpp` (316 lignes)

### À vérifier/modifier
- `src/CMakeLists.txt` - Retirer les références si présentes
- Documentation éventuelle mentionnant MpvFboItem

## Checklist de validation

- [ ] Fichiers `MpvFboItem.hpp` et `MpvFboItem.cpp` supprimés
- [ ] `CMakeLists.txt` mis à jour (si nécessaire)
- [ ] Aucune référence résiduelle à `MpvFboItem` dans le code
- [ ] Le projet compile sans erreur
- [ ] Tous les tests passent
- [ ] La lecture vidéo fonctionne (test manuel rapide)

## Notes

- **Effort estimé** : S (Small, < 30 min)
- **Impact** : Clarification du codebase, -415 lignes de code mort
- **Risque** : Nul (code jamais utilisé)

Cette tâche clôture définitivement la discussion sur le refactoring media du Plan 24 (tâche 24.20).
