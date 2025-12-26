# Plan 09 - Amélioration du Panel de Préférences

## Contexte
Le panel de préférences accessible depuis la Home Page présente plusieurs problèmes :
1. Le bouton "Fermer" / "Revenir à l'accueil" ne fonctionne pas
2. Problème de padding dans la box d'authentification Twitch
3. L'UI générale mériterait une modernisation

## Objectifs

### Bugs à corriger
1. **Bouton fermer non fonctionnel** : Le clic sur "Fermer" ou "Revenir à l'accueil" ne fait rien
2. **Padding manquant** : La box "Authentification Twitch" n'a pas de padding en bas (le bouton "Se déconnecter" touche le bord)

### Améliorations UI
- Moderniser l'apparence générale du panel
- Harmoniser les espacements et paddings
- Améliorer la lisibilité et l'organisation

### Nouvelles fonctionnalités (à définir lors de l'idéation)
- **Paramètres de cache** : Taille maximale du cache VOD (lié au plan-05)
- Autres paramètres à définir pendant la session d'idéation

## Spécifications

### Bug 1 - Bouton fermer
Le bouton doit fermer le panel et revenir à la Home Page.

```qml
// Comportement attendu
onClicked: {
    // Fermer le panel de préférences
    // Revenir à la vue précédente (Home)
}
```

### Bug 2 - Padding box authentification
```qml
// Ajouter un padding uniforme
Rectangle {
    // ...
    padding: 16  // ou utiliser des marges internes cohérentes
}
```

### Section Cache (nouvelle)
```
┌─────────────────────────────────────────────┐
│  Cache & Stockage                           │
│  ─────────────────────────────────────────  │
│                                             │
│  Taille maximale du cache                   │
│  [slider ════════════════════○] 10 GB       │
│                                             │
│  Espace utilisé : 2.3 GB / 10 GB            │
│  [████████████░░░░░░░░░░░░░░░░] 23%         │
│                                             │
│  [Vider le cache]                           │
└─────────────────────────────────────────────┘
```

## Session d'idéation (à compléter)

### Questions à explorer
- Quels autres paramètres seraient utiles ?
- Faut-il organiser les préférences en catégories/onglets ?
- Paramètres de lecture vidéo (qualité par défaut, etc.) ?
- Paramètres d'interface (thème, langue) ?
- Raccourcis clavier personnalisables ?

### Idées potentielles
- [ ] Paramètres de cache (taille max, vider le cache)
- [ ] Qualité vidéo par défaut
- [ ] Comportement de l'auto-play
- [ ] Notifications
- [ ] À compléter pendant l'implémentation...

## Fichiers concernés
- `src/ui/PreferencesView.qml`
- `src/ui/main.qml` (navigation)
- `src/core/Config.cpp` (stockage des préférences)

## Checklist de validation

### Bugs
- [ ] Le bouton "Fermer" ferme le panel
- [ ] Le bouton "Revenir à l'accueil" ramène à la Home
- [ ] La box authentification a un padding uniforme (haut, bas, gauche, droite)
- [ ] Le bouton "Se déconnecter" ne touche plus le bord

### UI générale
- [ ] Les espacements sont cohérents
- [ ] Les sections sont bien délimitées
- [ ] La police et les tailles sont harmonisées
- [ ] Le design est moderne et propre

### Nouvelles fonctionnalités
- [ ] Section "Cache & Stockage" présente
- [ ] Slider pour la taille max du cache
- [ ] Affichage de l'espace utilisé
- [ ] Bouton "Vider le cache"
- [ ] Les paramètres sont sauvegardés
- [ ] Les paramètres sont chargés au démarrage

### Tests
- [ ] Navigation fluide (ouvrir/fermer le panel)
- [ ] Pas de régression sur la déconnexion Twitch
- [ ] Les préférences persistent après redémarrage
