# Plan 16 - Mode Audio Uniquement

## Contexte

BluePlayer dispose déjà d'un toggle "Hardware/Software acceleration" dans le PlayerView pour gérer les performances. Le mode "Audio only" s'inscrit dans cette même logique : réduire l'utilisation des ressources en désactivant le rendu vidéo.

## Objectif

Ajouter une option "Audio only" dans le groupe de boutons performance existant, permettant d'écouter un stream sans afficher la vidéo (économie de bande passante et de ressources CPU/GPU).

## Spécifications

### UI - Extension du toggle existant

**Avant (2 états) :**
```
[HW] [SW]
```

**Après (3 états) :**
```
[HW] [SW] [Audio]
```

Ou sous forme de menu déroulant :
```
┌─────────────────┐
│  Mode lecture   │
│  ─────────────  │
│  ● Hardware     │
│  ○ Software     │
│  ○ Audio only   │
└─────────────────┘
```

### Comportement

#### Activation du mode Audio
1. L'utilisateur clique sur "Audio"
2. Le flux vidéo est désactivé (ou qualité "audio_only" sélectionnée)
3. Un placeholder visuel remplace la vidéo
4. L'audio continue normalement

#### Placeholder visuel

```
┌───────────────────────────────────────┐
│                                       │
│           🎵                          │
│                                       │
│      Mode audio uniquement            │
│                                       │
│   [Nom du streamer]                   │
│   [Titre du stream]                   │
│                                       │
└───────────────────────────────────────┘
```

Alternative : Afficher la thumbnail statique du stream.

#### Retour au mode vidéo
1. L'utilisateur clique sur "HW" ou "SW"
2. Le flux vidéo reprend
3. Le placeholder disparaît

### Implémentation technique

**Option 1 : Qualité "audio_only" (recommandée)**
- Twitch expose une qualité "audio_only" dans les streams HLS
- Utiliser le même mécanisme que le sélecteur de qualité (plan-11)
- Avantage : Économie réelle de bande passante

**Option 2 : Masquer le rendu vidéo**
- Garder le flux vidéo mais ne pas le rendre
- Avantage : Transition rapide
- Inconvénient : Bande passante toujours consommée

### Intégration avec Plan 11 (Qualité)

Si le plan-11 est implémenté avant :
- Le mode Audio peut être une qualité dans le sélecteur
- Ou rester un bouton séparé dans le groupe performance

## Fichiers concernés

### À modifier
- `src/ui/PlayerView.qml` - UI du toggle + placeholder
- `src/ui/components/PlayerControlBar.qml` - Bouton/toggle étendu
- `src/media/MpvQuickItem.cpp/hpp` - Gestion du mode audio

### API Twitch (si option 1)
- Utiliser la qualité "audio_only" du manifest HLS

## Dépendances

| Ce plan | Relation | Autre plan |
|---------|----------|------------|
| 16 | Bénéficie de | Plan 11 (Qualité) - Peut utiliser le même système |

## Checklist de validation

### UI
- [ ] Toggle étendu visible (HW/SW/Audio)
- [ ] État "Audio" clairement identifiable
- [ ] Placeholder affiché en mode audio
- [ ] Placeholder esthétique (thumbnail ou icône)
- [ ] Nom du streamer visible sur le placeholder
- [ ] Titre du stream visible sur le placeholder

### Fonctionnel
- [ ] Clic sur "Audio" active le mode
- [ ] L'audio fonctionne normalement
- [ ] Pas de rendu vidéo (vérifier CPU/GPU)
- [ ] Retour au mode vidéo fonctionne
- [ ] Transition fluide entre les modes

### Performance
- [ ] Réduction CPU visible en mode audio
- [ ] Réduction bande passante (si option 1)
- [ ] Pas de crash lors des transitions

### Cas particuliers
- [ ] Fonctionne en live
- [ ] Fonctionne en VOD
- [ ] Fonctionne avec le cache local
- [ ] État préservé si on change de qualité (plan-11)

### Tests
- [ ] L'app compile sans erreur
- [ ] Pas de régression sur les modes HW/SW existants
