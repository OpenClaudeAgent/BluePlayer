# Plan 37 - Raccourcis clavier globaux

## Contexte

Actuellement, les raccourcis clavier ne fonctionnent que lorsque BluePlayer est au premier plan. L'utilisateur doit cliquer sur l'application pour contrôler la lecture. Les touches média du clavier Mac (play/pause, volume) ne sont pas non plus captées.

## Objectif

Implémenter des raccourcis clavier globaux permettant de contrôler le player même quand BluePlayer n'est pas la fenêtre active, incluant le support des media keys.

## Specifications

### Comportement attendu

1. **Media keys du clavier**
   - Play/Pause : met en pause ou reprend la lecture
   - Volume Up/Down : ajuste le volume
   - Mute : coupe/rétablit le son
   - Fonctionnent même si une autre application est au premier plan

2. **Comportement système**
   - S'intégrer avec le Now Playing du système macOS
   - Respecter la priorité si une autre app utilise les media keys
   - Ne pas interférer avec les autres applications multimédia

3. **Feedback utilisateur**
   - Toast discret quand une action est déclenchée en arrière-plan
   - Optionnel : afficher le volume actuel lors du changement

4. **Préférences**
   - Option pour activer/désactiver les raccourcis globaux
   - Désactivés par défaut (pour ne pas surprendre l'utilisateur)
   - Réglage dans le panel Préférences

### Limitations V1

- Raccourcis non personnalisables (mapping fixe)
- Uniquement les media keys standard du Mac
- Pas de raccourcis globaux personnalisés (ex: Cmd+Shift+P)

### Considérations techniques

- Utiliser les APIs macOS pour capturer les media keys
- Gérer le cas où l'utilisateur n'a pas donné les permissions
- Respecter les guidelines macOS pour le comportement attendu

## Fichiers concernes

- Nouveau : `src/core/GlobalHotkeys.mm` - Capture des media keys (Objective-C++)
- Nouveau : `src/core/GlobalHotkeys.hpp` - Interface C++
- `src/media/MpvQuickItem.cpp` - Réception des commandes
- `src/ui/PreferencesView.qml` - Option d'activation

## Checklist de validation

### Media keys
- [ ] Play/Pause fonctionne en arrière-plan
- [ ] Volume Up/Down fonctionne en arrière-plan
- [ ] Mute fonctionne en arrière-plan

### Intégration système
- [ ] Les media keys fonctionnent avec BluePlayer non focusé
- [ ] Pas de conflit avec d'autres apps multimédia actives
- [ ] Le Now Playing macOS reflète l'état de BluePlayer

### Préférences
- [ ] L'option est présente dans les Préférences
- [ ] Désactivé par défaut
- [ ] Le changement de réglage est immédiat

### Non-régression
- [ ] Les raccourcis locaux fonctionnent toujours
- [ ] Le player fonctionne normalement
- [ ] L'application compile sans erreur
