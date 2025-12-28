# Plan 28 - Strategie de Navigation Globale

## Contexte

L'application BluePlayer dispose de plusieurs vues accessibles depuis l'ecran principal :

- **HomeView** : Vue par defaut avec les streams suivis
- **PreferencesView** : Panel de preferences avec son propre header
- **CacheManagerView** : Gestionnaire de replays avec son propre header
- **PlayerView** : Lecteur video en plein ecran

Actuellement, la navigation entre ces vues presente des problemes d'architecture et d'experience utilisateur.

## Probleme

### Architecture actuelle (main.qml)

1. **Boutons de navigation globaux** :
   - Positionnes en haut a droite de la fenetre avec `z: 10`
   - Bouton Replays (symbole fleche circulaire) et Bouton Preferences (symbole engrenage)
   - Visibles sur Home, Preferences et Cache
   - Chaque bouton a sa propre logique de toggle (active/inactive)

2. **Headers des panels** :
   - PreferencesView : Header avec bouton retour (fleche gauche) + titre "Preferences"
   - CacheManagerView : Header avec bouton retour + titre "Mes Replays" + marge droite de 100px (hack)

3. **Problemes identifies** :
   - **Overlap visuel** : Les boutons globaux (en haut a droite) se superposent aux headers des panels
   - **Workaround inconsistant** : CacheManagerView utilise une marge droite de 100px, mais pas PreferencesView
   - **Duplication de code** : Chaque panel reimplemente son propre header avec styles similaires
   - **Boutons non reutilisables** : Les boutons circulaires de main.qml sont du code inline, non des composants

### Impact sur l'experience utilisateur

- Confusion visuelle : deux zones de navigation peuvent se chevaucher
- Comportement inconsistant entre les differents panels
- Difficulte a etendre pour de nouveaux panels (Plan 12, etc.)

## Objectif

Concevoir et implementer une strategie de navigation coherente qui :

1. Elimine tout chevauchement visuel entre les elements de navigation
2. Unifie l'apparence et le comportement des headers de panels
3. Cree des composants reutilisables pour les futurs panels
4. Simplifie l'ajout de nouvelles vues sans duplication de code

## Comportement attendu

### Navigation depuis Home

- L'utilisateur voit une barre de navigation en haut avec acces aux Replays et Preferences
- Les boutons indiquent visuellement quel panel est actif (si un panel est ouvert)
- Un clic sur un bouton actif ferme le panel et retourne a Home

### Navigation dans un panel

- Le panel affiche un header clair avec :
  - Un moyen de retourner a la vue precedente
  - Le titre du panel
  - Aucun conflit visuel avec d'autres elements de navigation
- L'utilisateur comprend immediatement comment fermer le panel

### Transitions

- Les transitions entre vues sont fluides et coherentes
- Le feedback visuel est immediat (hover, active states)

## Composants a creer

### 1. Composant TopBar / NavigationBar

Un composant unique pour la barre de navigation superieure qui :
- Gere l'affichage des boutons de navigation globaux
- S'adapte selon la vue active (Home vs Panel)
- Peut afficher un bouton retour et un titre quand un panel est ouvert

### 2. Composant CircleButton (ou IconButton)

Un bouton circulaire reutilisable avec :
- Etat normal, hover et active
- Gestion du tooltip
- Style uniforme (taille, couleurs, border)

### 3. Composant PanelHeader

Un header standard pour tous les panels avec :
- Bouton retour a gauche
- Titre centre ou a gauche
- Zone d'actions optionnelle a droite
- Pas de conflit avec la navigation globale

## Strategies envisageables

### Option A : Navigation globale qui s'adapte

La barre de navigation globale reste visible mais son contenu change selon la vue :
- Sur Home : affiche les boutons Replays et Preferences
- Sur un panel : affiche un bouton retour + titre du panel

### Option B : Navigation globale cachee dans les panels

Quand un panel est ouvert :
- Les boutons globaux disparaissent (fade out)
- Le panel affiche son propre header avec bouton retour
- Les deux zones ne coexistent jamais

### Option C : Fusion des headers

Chaque panel "prend le controle" de la barre de navigation :
- Le bouton retour remplace les boutons normaux
- Le titre du panel s'affiche dans la TopBar
- Architecture type "vue maitre" qui controle la navigation

## Fichiers concernes

- `src/ui/main.qml` - Navigation globale et conteneurs de vues
- `src/ui/PreferencesView.qml` - Header du panel preferences
- `src/ui/CacheManagerView.qml` - Header du panel replays
- `src/ui/components/` - Nouveaux composants reutilisables a creer

## Dependances

| Plan | Relation |
|------|----------|
| Plan 12 (Preferences) | Ce plan clarifie l'architecture avant d'ajouter plus de contenu |
| Plan 25 (Audit UI/UX) | S'integre dans la reflexion design system |
| Plans futurs | Tout nouveau panel beneficiera de cette architecture |

## Sous-taches

- 28.1 - Analyse et choix de la strategie (A, B ou C)
- 28.2 - Creation du composant CircleButton (ou IconButton)
- 28.3 - Creation du composant TopBar / NavigationBar
- 28.4 - Refactoring de PreferencesView pour utiliser les nouveaux composants
- 28.5 - Refactoring de CacheManagerView pour utiliser les nouveaux composants
- 28.6 - Mise a jour de main.qml pour utiliser TopBar
- 28.7 - Tests et validation des transitions

## Priorite des sous-taches

| Priorite | Sous-tache | Dependances |
|----------|------------|-------------|
| 1 | 28.1 | Aucune |
| 2 | 28.2 | 28.1 |
| 3 | 28.3 | 28.1, 28.2 |
| 4 | 28.4 | 28.3 |
| 4 | 28.5 | 28.3 |
| 5 | 28.6 | 28.4, 28.5 |
| 6 | 28.7 | 28.6 |

## Checklist de validation

- [ ] Les boutons de navigation globaux et les headers de panels ne se chevauchent jamais
- [ ] PreferencesView et CacheManagerView utilisent le meme composant pour leur header
- [ ] Un composant CircleButton/IconButton reutilisable existe dans components/
- [ ] Un composant TopBar/NavigationBar centralise la logique de navigation
- [ ] Les transitions entre vues sont fluides et coherentes
- [ ] Le hack de marge droite dans CacheManagerView a ete supprime
- [ ] L'ajout d'un nouveau panel ne necessite pas de duplication de code header
- [ ] L'experience utilisateur est coherente entre toutes les vues
