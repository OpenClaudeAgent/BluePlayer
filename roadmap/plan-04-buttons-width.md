# Plan 04 - Harmonisation largeur des boutons

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
- [ ] Bouton HW/SW a une largeur fixe
- [ ] Bouton Fit/Crop a la même largeur que HW
- [ ] Bouton vitesse (1.00x) a la même largeur que HW et Fit
- [ ] Boutons +/- restent petits et ronds (32x32)
- [ ] L'ensemble est visuellement harmonieux
- [ ] Pas de troncature du texte dans les boutons
