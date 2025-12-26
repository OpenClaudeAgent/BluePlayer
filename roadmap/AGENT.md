# Agent Exécuteur - Roadmap BluePlayer

## Rôle

Cet agent automatise l'exécution des tâches de la roadmap en suivant un workflow structuré avec validation utilisateur.

## Workflow

### Phase 1 : Sélection de la tâche

1. Lire `roadmap/README.md`
2. Identifier la prochaine tâche avec statut "En attente" (en respectant les dépendances)
3. Afficher à l'utilisateur : "Prochaine tâche : [Nom de la tâche]. On y va ?"
4. Attendre confirmation

### Phase 2 : Préparation

1. Lire le fichier plan de la tâche (`plan-XX-*.md`)
2. Créer la branche Git dédiée (`feature/[nom]`)
3. Analyser les fichiers concernés
4. Définir un plan d'implémentation interne (todos)

### Phase 3 : Implémentation

1. Implémenter selon les spécifications du plan
2. Builder et vérifier qu'il n'y a pas d'erreurs
3. Tester si applicable

### Phase 4 : Validation utilisateur

1. Présenter la checklist à l'utilisateur :
   ```
   ## Validation - [Nom de la tâche]
   
   Merci de tester et valider chaque point :
   
   | # | Critère | Statut |
   |---|---------|--------|
   | 1 | [Point 1] | ? |
   | 2 | [Point 2] | ? |
   ...
   
   Tous les points sont validés ?
   ```

2. **Si NON** : 
   - Demander quel(s) point(s) pose(nt) problème
   - Corriger
   - Re-présenter la checklist
   - Répéter jusqu'à validation complète

3. **Si OUI** : Passer à la phase 5

### Phase 5 : Finalisation

1. **Mettre à jour le plan** (`plan-XX-*.md`) :
   - Cocher toutes les checkboxes de validation (`[x]`)
   - Si des fonctionnalités bonus ont été ajoutées, créer une section :
     ```markdown
     ## Bonus (ajouté lors de l'implémentation)
     
     - **[Nom fonctionnel]** : [Description fonctionnelle, sans code]
     ```

2. **Mettre à jour la roadmap** (`README.md`) :
   - Changer le statut de la tâche : `🔴 En attente` → `🟢 Terminé`
   - Ajouter une entrée dans l'historique :
     ```
     | [Date] | Tâche X terminée - [Description fonctionnelle courte] |
     ```

3. **Commit et merge** :
   ```bash
   git add -A
   git commit -m "feat([scope]): [description]"
   git checkout main
   git merge feature/[nom]
   ```

4. Confirmer à l'utilisateur : "Tâche X terminée et mergée sur main."

## Règles importantes

### Contenu fonctionnel uniquement

- La roadmap et les plans décrivent des **fonctionnalités**, pas du code
- Pas de snippets de code dans les plans (sauf section "Code de référence" initiale si utile)
- Les descriptions bonus sont fonctionnelles : "Le curseur disparaît automatiquement" et non `cursorShape: Qt.BlankCursor`

### Immutabilité des plans

Les plans sont immutables **sauf** :
- Les checkboxes de validation peuvent être cochées
- Une section "Bonus" peut être ajoutée pour documenter les fonctionnalités supplémentaires

### Respect des dépendances

Avant de commencer une tâche, vérifier dans la table des dépendances que toutes les tâches pré-requises sont terminées.

## Commande de lancement

Pour lancer l'agent, l'utilisateur dit :
- "Continue la roadmap"
- "Prochaine tâche"
- "On continue"

L'agent démarre alors à la Phase 1.
