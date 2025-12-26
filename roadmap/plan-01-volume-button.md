# Plan 01 - Bouton Volume

## Contexte
Quand on clique sur le bouton volume, deux actions se déclenchent simultanément :
- Le son est muté
- La barre de volume s'ouvre

Ce comportement n'est pas intuitif.

## Objectif
Séparer les actions du bouton volume pour un comportement plus naturel.

## Spécifications

### Comportement attendu
1. **Premier clic** : Ouvre la barre de volume (sans muter)
2. **Clic suivant (barre ouverte)** : Mute/unmute le son
3. **Slider de volume** : Permet d'ajuster le niveau sonore

### Logique
```
Si barre fermée:
    Clic → Ouvrir la barre
Si barre ouverte:
    Clic → Toggle mute
```

## Fichiers concernés
- `src/ui/components/PlayerControlBar.qml`

## Checklist de validation
- [x] ~~Premier clic sur le bouton volume ouvre la barre (sans muter)~~ Remplacé par hover
- [x] Hover sur le bouton/zone ouvre la barre de volume
- [x] Clic sur le bouton mute le son
- [x] Clic à nouveau unmute le son
- [x] Le slider de volume fonctionne correctement
- [x] La barre se ferme après un délai d'inactivité (1.5s)
- [x] Le comportement est intuitif et prévisible
- [x] Slider vertical (layout stable, pas de décalage)
- [x] Contrôles dans le bon sens (haut = +, bas = -)
