# Plan 12 - Amélioration du Panel de Préférences

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
- [ ] **Qualité vidéo par défaut** ⚠️ *Dépend de plan-11 (Sélection Qualité)*
- [ ] Comportement de l'auto-play
- [ ] Notifications
- [ ] À compléter pendant l'implémentation...

### Note sur la qualité par défaut

Cette fonctionnalité ne peut être implémentée qu'APRÈS le plan-11 (Sélection de la Qualité du Stream).

**Workflow prévu :**
1. Plan-11 implémente le sélecteur de qualité dans le player ✅
2. Plan-12 peut ensuite ajouter un paramètre "Qualité par défaut" dans les préférences ✅ (UI uniquement)
3. Ce paramètre sera automatiquement appliqué au démarrage de chaque stream ❌ (reporté)

> **Note d'implémentation (2025-12-28)** : Lors de l'implémentation, l'UI du dropdown a été créée mais la persistance et l'intégration avec TwitchService ont été jugées trop complexes pour ce plan. Ces fonctionnalités sont reportées au **Plan 29** dédié.

**UI envisagée :**
```
┌─────────────────────────────────────────────┐
│  Lecture                                    │
│  ─────────────────────────────────────────  │
│                                             │
│  Qualité par défaut                         │
│  [Auto ▼]                                   │
│  ├─ Auto (recommandé)                       │
│  ├─ 1080p60                                 │
│  ├─ 1080p                                   │
│  ├─ 720p60                                  │
│  └─ ...                                     │
│                                             │
└─────────────────────────────────────────────┘
```

## Fichiers concernés
- `src/ui/PreferencesView.qml`
- `src/ui/main.qml` (navigation)
- `src/core/Config.cpp` (stockage des préférences)

## Checklist de validation

### Bugs
- [x] Le bouton "Fermer" ferme le panel (bouton ← dans le header)
- [x] Le bouton "Revenir à l'accueil" ramène à la Home
- [x] La box authentification a un padding uniforme (haut, bas, gauche, droite)
- [x] Le bouton "Se déconnecter" ne touche plus le bord

### UI générale
- [x] Les espacements sont cohérents
- [x] Les sections sont bien délimitées
- [x] La police et les tailles sont harmonisées
- [x] Le design est moderne et propre

### Nouvelles fonctionnalités
- [x] Section "Cache & Stockage" présente
- [x] Stepper pour la taille max du cache (remplace slider)
- [x] Affichage de l'espace utilisé avec barre de progression
- [x] Bouton "Vider le cache" avec dialog de confirmation
- [ ] Les paramètres sont sauvegardés (qualité par défaut non persistée - backend à implémenter)
- [ ] Les paramètres sont chargés au démarrage (qualité par défaut non persistée)

### Tests
- [x] Navigation fluide (ouvrir/fermer le panel)
- [x] Pas de régression sur la déconnexion Twitch
- [ ] Les préférences persistent après redémarrage (partiel - cache OK, qualité non)

---

## Bonus (ajouté lors de l'implémentation)

- **Section Lecture avec qualité par défaut** : Dropdown pour choisir la qualité par défaut (UI uniquement, voir Plan 29 pour persistance)
- **Composant BlueDropdown** : Composant dropdown réutilisable ajouté au design system
- **Header moderne** : Bouton retour (←) avec titre, style glassmorphism
- **Composant PreferencesSection** : Composant inline pour les sections avec icône et titre
- **Indicateur de connexion animé** : Point vert pulsant quand connecté à Twitch
- **Barre de cache dynamique** : Se rafraîchit automatiquement quand la taille max change
- **Plan 28 créé** : Identification du problème d'overlap et plan pour stratégie navigation globale

## Reporté au Plan 29

Lors de l'analyse, les fonctionnalités suivantes ont été jugées trop complexes pour ce plan et reportées :

- **Persistance de la qualité par défaut** : Sauvegarder le choix de l'utilisateur (QSettings ou Config.cpp)
- **Intégration avec TwitchService** : Appliquer automatiquement la qualité par défaut à l'ouverture d'un stream
- **Synchronisation player ↔ préférences** : Mettre à jour les préférences si l'utilisateur change la qualité dans le player
