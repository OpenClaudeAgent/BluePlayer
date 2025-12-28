# Plan 35 - Créer des clips

## Contexte

Twitch permet aux utilisateurs de créer des "clips" pour capturer les moments forts d'un stream. Cette fonctionnalité est très utilisée pour partager des moments drôles, impressionnants ou mémorables. Actuellement, BluePlayer ne propose pas cette fonction.

## Objectif

Ajouter un bouton dans la barre de contrôle du player permettant de créer un clip du stream en cours, puis ouvrir le navigateur pour l'édition et le partage.

## Specifications

### Comportement attendu

1. **Bouton dans la barre de contrôle**
   - Icône reconnaissable (ciseaux ou icône clip Twitch)
   - Position : dans la zone des actions secondaires de la barre de contrôle
   - Visible uniquement sur les streams live (pas sur les VOD déjà enregistrées)

2. **Création du clip**
   - Un clic sur le bouton appelle l'API Twitch pour créer le clip
   - Feedback visuel pendant la création (loader, toast "Création en cours...")
   - L'API retourne un ID de clip et une URL d'édition

3. **Ouverture du navigateur**
   - Après création réussie, ouvrir l'URL d'édition dans le navigateur par défaut
   - L'utilisateur peut alors découper, titrer et partager le clip sur Twitch
   - Cette étape est obligatoire (limitation Twitch : pas d'édition in-app)

4. **Gestion des erreurs**
   - Stream non éligible aux clips : message explicatif
   - Échec de création : afficher l'erreur
   - Utilisateur non connecté : inviter à se connecter

### Limitations

- L'édition du clip se fait obligatoirement sur Twitch (navigateur)
- L'utilisateur doit être connecté pour créer des clips
- Certains streamers désactivent les clips

## Fichiers concernes

- `src/ui/components/PlayerControlBar.qml` - Ajout du bouton clip
- `src/api/twitch/TwitchService.cpp` - Appel API création de clip
- Nouveau : `src/core/SystemUtils.cpp/.hpp` - Ouverture URL dans navigateur (si n'existe pas)

## Checklist de validation

### Bouton
- [ ] Le bouton clip est visible dans la barre de contrôle
- [ ] L'icône est reconnaissable et cohérente avec le design
- [ ] Le bouton n'apparaît que sur les streams live

### Création
- [ ] Un clic déclenche la création du clip via l'API
- [ ] Un feedback visuel indique que la création est en cours
- [ ] Le toast de succès s'affiche après création

### Navigation
- [ ] L'URL d'édition s'ouvre dans le navigateur par défaut
- [ ] L'utilisateur peut éditer et partager le clip sur Twitch

### Gestion des erreurs
- [ ] Les erreurs API sont affichées clairement
- [ ] Le cas "clips désactivés" est géré
- [ ] Le cas "non connecté" invite à se connecter

### Non-régression
- [ ] La barre de contrôle fonctionne normalement
- [ ] Les autres boutons ne sont pas affectés
- [ ] L'application compile sans erreur
