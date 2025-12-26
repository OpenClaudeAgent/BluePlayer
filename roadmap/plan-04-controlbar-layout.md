# Plan 04 - Layout barre de contrôle

## Contexte
- La seekbar est plus haute que les boutons
- Le bouton LIVE/VOD est décorrélé visuellement du reste
- L'espacement entre le bouton fullscreen et le bouton volume n'est pas optimal
- Les éléments ne sont pas bien groupés logiquement

## Objectif
Réorganiser le layout de la barre de contrôle pour un design plus cohérent.

## Spécifications

### Layout attendu (de gauche à droite)

```
[Play/Pause] | [══════ SEEKBAR ══════] [LIVE] | [-][1.00x][+] [HW] [Fit] [Vol] [FS]
     ^                   ^                ^              ^
   Gauche             Centre          Adjacent      Groupe droite
```

### Organisation
1. **Gauche** : Bouton Play/Pause
2. **Centre** : Seekbar (prend tout l'espace disponible)
3. **Adjacent seekbar** : Bouton LIVE/VOD
4. **Droite (groupés, alignés)** :
   - Contrôles vitesse (-/speed/+)
   - Bouton HW
   - Bouton Fit
   - Bouton Volume (avec slider extensible)
   - Bouton Fullscreen

### Alignement vertical
- Tous les éléments doivent avoir la même hauteur visuelle (32px)
- La seekbar doit être alignée verticalement avec les boutons
- Le bouton LIVE doit être aligné avec la seekbar

### Espacement
- Espacement cohérent entre les groupes
- Espacement réduit au sein des groupes

## Fichiers concernés
- `src/ui/components/PlayerControlBar.qml`

## Checklist de validation
- [x] Play/Pause est à gauche
- [x] Seekbar prend l'espace central disponible
- [x] Bouton LIVE est adjacent à la seekbar (à droite)
- [x] Groupe de droite : vitesse, HW, Fit, Volume, Fullscreen
- [x] Tous les éléments sont alignés verticalement
- [x] La seekbar a la même hauteur visuelle que les boutons
- [x] L'espacement est cohérent entre tous les éléments
- [x] L'espacement est correct entre Volume et Fullscreen
- [x] Le design est propre et professionnel

## Bonus (ajouté lors de l'implémentation)

- **Bouton LIVE compact** : Bouton rond minimaliste avec dot pulsant (sans texte), cliquable pour resynchroniser le direct
- **Boutons HW/Fit compacts** : Largeur réduite pour un design plus équilibré
- **Timer intelligent** : Affichage position/durée sur deux lignes, masqué automatiquement en mode live
- **Police monospace** : Chiffres à largeur fixe pour éviter les décalages visuels
