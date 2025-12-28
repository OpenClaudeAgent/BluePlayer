# Plan 34 - Points de chaîne

## Contexte

Les viewers accumulent des "Channel Points" sur Twitch en regardant des streams. Ces points peuvent être utilisés pour débloquer des récompenses spécifiques à chaque chaîne (highlight message, emotes, actions personnalisées). Actuellement, BluePlayer n'affiche pas ces points et ne permet pas leur utilisation.

## Objectif

Afficher le solde de points de chaîne de l'utilisateur et permettre l'utilisation de ces points pour les récompenses disponibles.

## Specifications

### Comportement attendu

1. **Affichage du solde**
   - Afficher le nombre de points actuels pour la chaîne en cours
   - Position : dans le panel chat ou près du champ de saisie
   - Icône représentative (icône Twitch ou personnalisée de la chaîne)

2. **Liste des récompenses**
   - Afficher les récompenses disponibles sur la chaîne
   - Montrer le coût en points de chaque récompense
   - Indiquer si une récompense est accessible (assez de points)

3. **Utilisation des points**
   - Permettre de cliquer sur une récompense pour l'utiliser
   - Confirmation avant utilisation
   - Feedback visuel après utilisation (succès/échec)
   - Note : certaines récompenses nécessitent une saisie de texte

4. **Cas particuliers**
   - Utilisateur non connecté : ne pas afficher la section
   - Chaîne sans points activés : ne pas afficher la section
   - Chaîne en mode sub-only rewards : indiquer les restrictions

### Limitations connues

- L'API Twitch peut limiter certaines actions (à vérifier)
- Certaines récompenses custom peuvent ne pas être supportées

## Fichiers concernes

- `src/ui/components/ChatPanel.qml` - Affichage du solde
- Nouveau : `src/ui/components/ChannelPointsPanel.qml` - Panel des récompenses
- `src/api/twitch/TwitchService.cpp` - Appels API points de chaîne

## Checklist de validation

### Affichage du solde
- [ ] Le solde de points s'affiche pour la chaîne en cours
- [ ] L'icône des points est visible et reconnaissable
- [ ] Le solde se met à jour après utilisation

### Récompenses
- [ ] La liste des récompenses disponibles s'affiche
- [ ] Le coût de chaque récompense est visible
- [ ] Les récompenses inaccessibles sont visuellement distinctes

### Utilisation
- [ ] Cliquer sur une récompense ouvre une confirmation
- [ ] L'utilisation déclenche l'action sur Twitch
- [ ] Le solde est mis à jour après utilisation
- [ ] Les erreurs sont affichées clairement

### Non-régression
- [ ] Le chat fonctionne normalement
- [ ] L'envoi de messages n'est pas affecté
- [ ] L'application compile sans erreur
