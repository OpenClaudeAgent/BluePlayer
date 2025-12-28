# Plan 16 - Mode Audio Uniquement

## Contexte

BluePlayer dispose déjà d'un toggle "Hardware/Software acceleration" dans le PlayerView pour gérer les performances. Le mode "Audio only" s'inscrit dans cette même logique : réduire l'utilisation des ressources en désactivant le rendu vidéo.

> **Note (Plan 11 implémenté)** : Le sélecteur de qualité (Plan 11) est maintenant disponible. Si Twitch expose une qualité "audio_only" dans le manifest HLS, elle apparaît automatiquement dans la liste des qualités. Ce plan se concentre désormais sur l'**amélioration de l'expérience audio** : placeholder visuel, indicateur dédié, et optimisations spécifiques au mode audio.

## Objectif

~~Ajouter une option "Audio only" dans le groupe de boutons performance existant.~~

**Objectif révisé** : Améliorer l'expérience du mode audio existant (accessible via le sélecteur de qualité) en ajoutant un placeholder visuel élégant et des optimisations de ressources.

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
- [x] ~~Toggle étendu visible (HW/SW/Audio)~~ Intégré au sélecteur de qualité existant
- [x] État "Audio" clairement identifiable (icône 🔊)
- [x] Placeholder affiché en mode audio
- [x] Placeholder esthétique (thumbnail + icône 🎵 avec animation)
- [x] Nom du streamer visible sur le placeholder
- [x] Titre du stream visible sur le placeholder

### Fonctionnel
- [x] Sélection "Audio" via le sélecteur de qualité
- [x] L'audio fonctionne normalement
- [x] Placeholder masque le rendu vidéo
- [x] Retour au mode vidéo fonctionne
- [x] Transition fluide entre les modes

### Performance
- [x] Économie bande passante (qualité audio_only Twitch)
- [x] Pas de crash lors des transitions

### Cas particuliers
- [x] Fonctionne en live
- [x] Fonctionne avec le cache local (badge qualité formaté)

### Tests
- [x] L'app compile sans erreur
- [x] Pas de régression sur les modes HW/SW existants

---

## Bonus (ajouté lors de l'implémentation)

### Labels de qualité normalisés
- **Fonction centralisée `formatQuality()`** : Transforme les labels techniques en labels lisibles
- **Fonction centralisée `isAudioQuality()`** : Détection du mode audio sans duplication
- **Mapping des labels** : "audio_only" → "Audio", "1080p60" → "1080p 60fps", "chunked" → "Source"
- **Application uniforme** : Sélecteur qualité, badges My Replays, toasts

### Toast redesigné
- **Forme pill moderne** avec coins arrondis dynamiques
- **Taille dynamique** selon le contenu du texte
- **Fond noir 75% opacité** pour lisibilité sur vidéo
- **Icône info stylée** (cercle bleu accent avec "i")
