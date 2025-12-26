# Plan 03 - Harmonisation largeur des boutons

## Contexte
Les boutons de la barre de contrôle n'ont pas des largeurs cohérentes :
- Bouton **HW/SW** : largeur X
- Bouton **Fit/Crop** : largeur Y (différente)
- Bouton **vitesse (1.00x)** : largeur Z (différente)
- Boutons **+/-** : OK (petits et ronds)

## Objectif
Harmoniser la largeur des boutons HW, Fit et vitesse.

## Spécifications

### Largeurs attendues
- **Boutons +/-** : 32px (petits, ronds) - **conserver tel quel**
- **Boutons HW, Fit, Vitesse** : même largeur fixe (ex: 60px ou 64px)

### Style cohérent
```qml
// Largeur commune pour les boutons principaux
property int chipWidth: 60
property int chipHeight: 32
property int chipRadius: 16
```

## Fichiers concernés
- `src/ui/components/PlayerControlBar.qml`

## Checklist de validation
- [x] Bouton HW/SW a une largeur fixe
- [x] Bouton Fit/Crop a la même largeur que HW
- [x] Bouton vitesse (1.00x) a la même largeur que HW et Fit
- [x] Boutons +/- restent petits et ronds (32x32)
- [x] L'ensemble est visuellement harmonieux
- [x] Pas de troncature du texte dans les boutons

## Bonus (ajouté lors de l'implémentation)

### Style uniforme des boutons

Les boutons HW/SW et Fit/Crop avaient des inconsistances visuelles :
- HW actif : fond bleu + bordure bleue / SW : fond normal
- Crop actif : fond violet + bordure violette / Fit : fond normal

**Correction** : Tous les boutons ont maintenant un style identique quel que soit leur état. Seul le texte change pour indiquer l'état actuel.

### Checklist bonus
- [x] Bouton HW/SW garde la même couleur quel que soit l'état
- [x] Bouton Fit/Crop garde la même couleur quel que soit l'état
- [x] Les textes et infobulles restent cohérents
