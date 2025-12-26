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
- [ ] Premier clic sur le bouton volume ouvre la barre (sans muter)
- [ ] Deuxième clic (barre ouverte) mute le son
- [ ] Troisième clic (barre ouverte) unmute le son
- [ ] Le slider de volume fonctionne correctement
- [ ] La barre se ferme après un délai d'inactivité
- [ ] Le comportement est intuitif et prévisible
