# Catalogue des Scénarios de Test

Ce document répertorie tous les scénarios de test fonctionnels UI pour BluePlayer, organisés par composant.

## Légende

| Statut | Signification |
|--------|---------------|
| ✅ | Implémenté et passant |
| 🔄 | En cours d'implémentation |
| 📋 | Planifié |
| ⏸️ | Bloqué / En attente |

---

## PlayerControlBar

Barre de contrôle du lecteur vidéo avec play/pause, volume, seek, vitesse, etc.

### Play/Pause

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| PCB-001 | Clic sur Play → émet `playPauseClicked` | ✅ | Haute |
| PCB-002 | Double-clic sur Play → émet le signal 2 fois | ✅ | Moyenne |
| PCB-003 | Icône change de ▶ à ⏸ quand `playing=true` | ✅ | Haute |
| PCB-004 | Icône affiche "..." quand `buffering=true` | 📋 | Moyenne |
| PCB-005 | Raccourci Espace → toggle play/pause | 📋 | Haute |

### Volume

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| VOL-001 | Clic sur bouton volume → émet `muteClicked` | ✅ | Haute |
| VOL-002 | Drag slider volume → émet `volumeRequested` | ✅ | Haute |
| VOL-003 | Hover sur bouton → affiche slider popup | 📋 | Moyenne |
| VOL-004 | Icône change quand `muted=true` | ✅ | Haute |
| VOL-005 | Slider respecte min/max (0.0 - 1.0) | 📋 | Moyenne |
| VOL-006 | Raccourci M → toggle mute | 📋 | Haute |
| VOL-007 | Molette souris sur slider → ajuste volume | 📋 | Basse |

### Vitesse de lecture

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| SPD-001 | Clic sur + → augmente vitesse de 0.25 | ✅ | Haute |
| SPD-002 | Clic sur - → diminue vitesse de 0.25 | ✅ | Haute |
| SPD-003 | Clic sur chip vitesse → reset à 1.0x | ✅ | Haute |
| SPD-004 | Vitesse max limitée à 3.0x | ✅ | Moyenne |
| SPD-005 | Vitesse min limitée à 0.25x | ✅ | Moyenne |
| SPD-006 | Affichage correct du label (ex: "1.50x") | 📋 | Moyenne |
| SPD-007 | Raccourci R → reset vitesse | 📋 | Moyenne |

### Timeline / Seek

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| SEK-001 | Drag timeline → émet `seekRequested` | 📋 | Haute |
| SEK-002 | Clic sur timeline → seek à la position | 📋 | Haute |
| SEK-003 | Affichage position actuelle (mm:ss) | 📋 | Moyenne |
| SEK-004 | Affichage durée totale | 📋 | Moyenne |
| SEK-005 | Mode Live : slider toujours à droite | 📋 | Haute |
| SEK-006 | Mode VOD : slider suit la position | 📋 | Haute |
| SEK-007 | Hover timeline → preview de la position | 📋 | Basse |

### Live / VOD Mode

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| LIV-001 | Indicateur Live visible en mode live | 📋 | Haute |
| LIV-002 | Indicateur Live masqué en mode replay | 📋 | Haute |
| LIV-003 | Clic sur indicateur Live → retour au direct | 📋 | Haute |
| LIV-004 | Animation pulsante quand au live edge | 📋 | Basse |
| LIV-005 | Couleur différente quand derrière le live | 📋 | Moyenne |

### Autres contrôles

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| CTL-001 | Clic fullscreen → émet `fullscreenClicked` | ✅ | Haute |
| CTL-002 | Clic chat toggle → émet `chatToggleClicked` | ✅ | Haute |
| CTL-003 | Bouton chat change couleur quand visible | ✅ | Moyenne |
| CTL-004 | Toggle HW/SW → émet signal | 📋 | Moyenne |
| CTL-005 | Toggle Fit/Crop → émet signal | 📋 | Moyenne |
| CTL-006 | Auto-hide après inactivité | 📋 | Moyenne |
| CTL-007 | Réapparaît au mouvement souris | 📋 | Moyenne |

---

## HomeView

Vue d'accueil avec les streams, catégories, et recherche.

### Navigation Streams

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| HOM-001 | Clic sur StreamCard → ouvre le stream | 📋 | Haute |
| HOM-002 | Scroll horizontal dans une section | 📋 | Haute |
| HOM-003 | Boutons flèches gauche/droite fonctionnent | 📋 | Moyenne |
| HOM-004 | Hover sur card → affiche overlay info | 📋 | Moyenne |
| HOM-005 | Affichage du nombre de viewers | 📋 | Moyenne |

### Catégories

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| CAT-001 | Clic sur CategoryCard → filtre par catégorie | 📋 | Haute |
| CAT-002 | Scroll horizontal catégories | 📋 | Moyenne |
| CAT-003 | Affichage correct des images | 📋 | Moyenne |

### Recherche

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| SRC-001 | Saisie texte → filtre les résultats | 📋 | Haute |
| SRC-002 | Entrée → lance la recherche | 📋 | Haute |
| SRC-003 | Effacer → réinitialise la liste | 📋 | Moyenne |
| SRC-004 | Résultats vides → message approprié | 📋 | Moyenne |
| SRC-005 | Debounce sur la saisie | 📋 | Basse |

### États de chargement

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| LOD-001 | Spinner visible pendant le chargement | 📋 | Moyenne |
| LOD-002 | Message d'erreur si échec réseau | 📋 | Haute |
| LOD-003 | Pull-to-refresh fonctionne | 📋 | Moyenne |
| LOD-004 | Skeleton loading pour les cards | 📋 | Basse |

---

## VideoPlayer

Lecteur vidéo local avec sélection de fichier.

### Sélection de fichier

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| VPL-001 | Clic "Parcourir" → ouvre FileDialog | 📋 | Haute |
| VPL-002 | Sélection fichier → met à jour le chemin | 📋 | Haute |
| VPL-003 | Annulation → pas de changement | 📋 | Moyenne |
| VPL-004 | Filtre par extensions vidéo | 📋 | Moyenne |

### Contrôles de lecture

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| VPL-010 | Bouton Play activé si fichier sélectionné | 📋 | Haute |
| VPL-011 | Bouton Play désactivé si pas de fichier | 📋 | Haute |
| VPL-012 | Clic Play → démarre la lecture | 📋 | Haute |
| VPL-013 | Clic Stop → arrête la lecture | 📋 | Haute |
| VPL-014 | Message statut mis à jour | 📋 | Moyenne |

---

## LoginView

Vue de connexion OAuth Twitch.

### Flux OAuth

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| LOG-001 | Clic "Se connecter" → ouvre navigateur | 📋 | Haute |
| LOG-002 | Token valide → redirige vers Home | 📋 | Haute |
| LOG-003 | Token invalide → affiche erreur | 📋 | Haute |
| LOG-004 | Bouton déconnexion → supprime token | 📋 | Haute |

---

## Navigation Inter-Vues

Tests de navigation globale dans l'application.

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| NAV-001 | Home → PlayerView via StreamCard | 📋 | Haute |
| NAV-002 | PlayerView → Home via bouton retour | 📋 | Haute |
| NAV-003 | Navigation préserve l'état | 📋 | Moyenne |
| NAV-004 | Deep link vers un stream | 📋 | Basse |

---

## Raccourcis Clavier Globaux

| ID | Scénario | Statut | Priorité |
|----|----------|--------|----------|
| KEY-001 | Espace → Play/Pause | 📋 | Haute |
| KEY-002 | M → Mute/Unmute | 📋 | Haute |
| KEY-003 | F → Fullscreen | 📋 | Haute |
| KEY-004 | Échap → Quitter fullscreen | 📋 | Haute |
| KEY-005 | C → Toggle chat | 📋 | Moyenne |
| KEY-006 | R → Reset vitesse | 📋 | Moyenne |
| KEY-007 | V → Toggle Fit/Crop | 📋 | Basse |
| KEY-008 | Flèches ← → → Seek ±10s | 📋 | Moyenne |
| KEY-009 | Flèches ↑ ↓ → Volume ±5% | 📋 | Moyenne |

---

## Métriques de couverture

### Par composant

| Composant | Scénarios total | Implémentés | Couverture |
|-----------|-----------------|-------------|------------|
| PlayerControlBar | 32 | 12 | 37% |
| HomeView | 14 | 0 | 0% |
| VideoPlayer | 9 | 0 | 0% |
| LoginView | 4 | 0 | 0% |
| Navigation | 4 | 0 | 0% |
| Raccourcis | 9 | 0 | 0% |
| **Total** | **72** | **12** | **17%** |

### Par priorité

| Priorité | Total | Implémentés |
|----------|-------|-------------|
| Haute | 38 | 8 |
| Moyenne | 26 | 4 |
| Basse | 8 | 0 |

---

## Comment ajouter un scénario

1. Identifier le composant concerné
2. Définir un ID unique (PREFIX-XXX)
3. Décrire le scénario en une phrase
4. Assigner une priorité
5. Ajouter au tableau approprié
6. Implémenter le test dans le fichier `tst_*.qml` correspondant
7. Mettre à jour le statut

### Convention d'ID

| Préfixe | Composant |
|---------|-----------|
| PCB | PlayerControlBar |
| VOL | Volume controls |
| SPD | Speed controls |
| SEK | Seek/Timeline |
| LIV | Live/VOD mode |
| CTL | Other controls |
| HOM | HomeView |
| CAT | Categories |
| SRC | Search |
| LOD | Loading states |
| VPL | VideoPlayer |
| LOG | LoginView |
| NAV | Navigation |
| KEY | Keyboard shortcuts |
