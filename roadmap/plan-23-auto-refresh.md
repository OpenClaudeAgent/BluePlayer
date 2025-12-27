# Plan 23 - Rafraîchissement automatique de la Home

## Contexte

Actuellement, les données de la page d'accueil (streams live, clips populaires) sont chargées une seule fois au démarrage de l'application. Si un streamer passe en live ou arrête son stream pendant que l'utilisateur navigue, l'interface affiche des données obsolètes. Cliquer sur un stream qui n'est plus live provoque un comportement incohérent.

## Objectif

Garder les données de la Home à jour automatiquement pour que l'utilisateur voie toujours l'état réel des streams.

## Comportement attendu

### 1. Rafraîchissement automatique toutes les 30 secondes

**Ce qui est rafraîchi :**
- Les streams live (suivis et recommandés)
- Les clips populaires

**Ce qui n'est PAS rafraîchi :**
- Les catégories (changent rarement)

**Ce que l'utilisateur voit :**
- Rien de visible pendant le refresh automatique (silencieux)
- Les cartes de streams se mettent à jour si un streamer passe offline/online
- Le nombre de viewers se met à jour

### 2. Rafraîchissement au retour de l'application

**Déclencheur :**
- L'utilisateur revient sur l'app après l'avoir mise en arrière-plan

**Ce que l'utilisateur voit :**
- Un petit indicateur de chargement discret (spinner) apparaît brièvement
- Les données se mettent à jour
- Le spinner disparaît avec un fondu

### 3. Indicateur visuel de refresh

**Apparence :**
- Petit spinner circulaire discret
- Positionné en haut de la page (près de la barre de recherche)
- Style cohérent avec le thème de l'app

**Quand il apparaît :**
- Au retour de l'app au premier plan
- Pendant un pull-to-refresh manuel (optionnel)

**Quand il disparaît :**
- Dès que les données sont chargées
- Avec une animation de fondu (pas de disparition brutale)

### 4. Gestion quand un stream n'est plus live

**Scénario :**
1. L'utilisateur voit un stream "MisterMV - 15 000 viewers" sur la Home
2. MisterMV arrête son stream
3. L'utilisateur clique sur la carte
4. Le stream n'existe plus

**Ce que l'utilisateur voit :**
1. Tentative de lecture qui échoue
2. Retour automatique à la page d'accueil
3. Refresh immédiat des données
4. La carte de MisterMV a disparu (il n'est plus live)

### 5. Pull-to-refresh (optionnel)

**Geste :**
- L'utilisateur tire vers le bas sur la Home

**Ce qui se passe :**
- Le spinner apparaît
- Les données se rafraîchissent
- Le spinner disparaît

## Sous-tâches

- 23.1 - Refresh automatique toutes les 30 secondes
- 23.2 - Refresh au retour au premier plan
- 23.3 - Indicateur visuel (spinner discret)
- 23.4 - Retour Home + refresh si stream plus disponible
- 23.5 - Pull-to-refresh (optionnel)

## Priorité des sous-tâches

| Priorité | Sous-tâche | Dépendances |
|----------|------------|-------------|
| 1 | 23.1 - Refresh auto 30s | Aucune |
| 2 | 23.4 - Gestion stream offline | 23.1 |
| 3 | 23.2 - Refresh retour premier plan | 23.1 |
| 4 | 23.3 - Spinner discret | 23.2 |
| 5 | 23.5 - Pull-to-refresh | 23.3 |

## Checklist de validation

- [ ] Les streams suivis se rafraîchissent toutes les 30 secondes
- [ ] Les clips populaires se rafraîchissent toutes les 30 secondes
- [ ] Quand l'app revient au premier plan, les données se mettent à jour
- [ ] Un spinner discret apparaît pendant le refresh au retour
- [ ] Le spinner disparaît avec un fondu doux
- [ ] Cliquer sur un stream terminé ramène à la Home
- [ ] Après retour à la Home, les données sont à jour
- [ ] Le refresh automatique est silencieux (pas de spinner)
- [ ] Pull-to-refresh fonctionne (optionnel)
