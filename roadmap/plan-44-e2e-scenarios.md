# Plan 44 - Scenarios E2E - Couverture Features

## Contexte

BluePlayer dispose d'une infrastructure E2E fonctionnelle (Plan 40) avec 13 tests de base couvrant :
- L'ouverture d'un stream live (scénario `open-stream`)
- La page de login (scénario `login-view`)

Cependant, 23 plans ont été implémentés (v0.1.0 à v0.29.0) avec de nombreuses fonctionnalités qui ne sont pas encore couvertes par des tests E2E. Cette absence de tests expose l'application à des régressions non détectées lors des futures évolutions.

## Objectif

Créer une suite de scénarios E2E qui couvrent les flux utilisateur critiques des features implémentées :
1. Valider les parcours utilisateur complets
2. Détecter les régressions avant les releases
3. Documenter le comportement attendu de chaque feature

---

## Scenarios proposes

### Groupe A : Lecture Video (Priorite Haute)

| # | Scenario | Feature testee | Priorite |
|---|----------|----------------|----------|
| A.1 | `search-stream` | Plan 6 - Recherche | Haute |
| A.2 | `play-cached-vod` | Plan 5 - Cache/VOD | Haute |
| A.3 | `quality-switch` | Plan 11 - Qualite | Haute |
| A.4 | `resume-playback` | Plan 15 - Historique VOD | Haute |
| A.5 | `audio-only-mode` | Plan 16 - Audio Only | Haute |

---

#### A.1 - Scenario `search-stream`

**Feature testee :** Plan 6 - Barre de recherche Twitch

**Flux utilisateur :**
1. L'utilisateur est sur la Home (authentifie)
2. L'utilisateur tape dans la barre de recherche
3. Les resultats apparaissent apres debounce (400ms)
4. L'utilisateur clique sur un streamer live
5. Le PlayerView s'ouvre avec le stream

**Assertions :**
- [ ] La barre de recherche est visible et focusable
- [ ] Les resultats s'affichent avec les bons streamers
- [ ] Le badge "LIVE" est visible pour les streams actifs
- [ ] Le clic sur un resultat navigue vers le player
- [ ] Le stream commence a charger

**Context :** `authenticated`

---

#### A.2 - Scenario `play-cached-vod`

**Feature testee :** Plan 5 - Gestion Cache/VOD

**Flux utilisateur :**
1. L'utilisateur est sur la Home
2. L'utilisateur clique sur le bouton "Replays" (icone ↺)
3. Le CacheManagerView s'ouvre
4. L'utilisateur voit une liste de VOD cachees (mockees)
5. L'utilisateur clique sur une VOD
6. Le PlayerView s'ouvre en mode replay

**Assertions :**
- [ ] Le bouton Replays est visible sur la Home
- [ ] CacheManagerView affiche la liste des VOD
- [ ] Les metadonnees (streamer, duree, taille) sont visibles
- [ ] Le clic sur une VOD ouvre le player
- [ ] Le mode est "replay" (pas "live")
- [ ] La SeekBar commence a 0

**Context :** `authenticated` + fixtures VOD mockees

---

#### A.3 - Scenario `quality-switch`

**Feature testee :** Plan 11 - Selecteur Qualite Stream

**Flux utilisateur :**
1. L'utilisateur regarde un stream (PlayerView)
2. L'utilisateur clique sur le bouton qualite (HD/SD)
3. Le popup de selection s'ouvre
4. L'utilisateur selectionne une qualite differente
5. Le stream change de qualite

**Assertions :**
- [ ] Le bouton qualite affiche HD ou SD
- [ ] Le popup liste les qualites disponibles
- [ ] La qualite actuelle est mise en evidence
- [ ] Le changement de qualite fonctionne sans interruption
- [ ] Le bouton affiche la nouvelle qualite

**Context :** `authenticated` + stream avec plusieurs qualites

---

#### A.4 - Scenario `resume-playback`

**Feature testee :** Plan 15 - Historique de visionnage

**Flux utilisateur :**
1. L'utilisateur ouvre une VOD partiellement vue
2. La lecture reprend automatiquement a la derniere position
3. La progression est visible dans l'UI

**Assertions :**
- [ ] La VOD reprend a la position sauvegardee
- [ ] La barre de progression dans CacheManagerView reflete l'avancement
- [ ] Le label "Vu il y a X jours" est affiche

**Context :** `authenticated` + VOD avec progression sauvegardee

---

#### A.5 - Scenario `audio-only-mode`

**Feature testee :** Plan 16 - Mode Audio uniquement

**Flux utilisateur :**
1. L'utilisateur regarde un stream
2. L'utilisateur ouvre le selecteur qualite
3. L'utilisateur selectionne "Audio"
4. Le placeholder audio s'affiche
5. L'audio continue de jouer

**Assertions :**
- [ ] L'option "Audio" est disponible dans le selecteur
- [ ] Le placeholder avec icone 🎵 s'affiche
- [ ] Le nom du streamer et titre sont visibles
- [ ] L'audio fonctionne normalement

**Context :** `authenticated` + stream avec qualite audio_only

---

### Groupe B : Chat et Interactions (Priorite Haute)

| # | Scenario | Feature testee | Priorite |
|---|----------|----------------|----------|
| B.1 | `chat-open-close` | Plan 7 - Chat | Haute |
| B.2 | `chat-messages` | Plan 7 - Chat | Moyenne |

---

#### B.1 - Scenario `chat-open-close`

**Feature testee :** Plan 7 - Chat Twitch integre

**Flux utilisateur :**
1. L'utilisateur regarde un stream live
2. L'utilisateur clique sur le bouton chat
3. Le panneau chat s'ouvre a droite
4. L'utilisateur voit les messages
5. L'utilisateur ferme le chat

**Assertions :**
- [ ] Le bouton chat est visible dans la control bar
- [ ] Le panneau chat s'ouvre au clic
- [ ] Le header affiche le nom du channel
- [ ] Les messages s'affichent (usernames colores)
- [ ] Le bouton chat est masque en mode VOD

**Context :** `authenticated` + mock IRC

---

#### B.2 - Scenario `chat-messages`

**Feature testee :** Plan 7 - Parsing messages, emotes, badges

**Flux utilisateur :**
1. Le chat est ouvert sur un stream live
2. Des messages arrivent (mockes)
3. Les badges et emotes s'affichent

**Assertions :**
- [ ] Les usernames ont la bonne couleur
- [ ] Les badges (mod, sub, vip) s'affichent comme icones
- [ ] Les emotes Twitch s'affichent comme images
- [ ] Le scroll automatique fonctionne
- [ ] La limite de 500 messages est respectee

**Context :** `authenticated` + mock IRC avec messages

---

### Groupe C : Controles Player (Priorite Moyenne)

| # | Scenario | Feature testee | Priorite |
|---|----------|----------------|----------|
| C.1 | `volume-control` | Plan 1 - Volume | Moyenne |
| C.2 | `fullscreen-toggle` | Plan 2 - Fullscreen | Moyenne |
| C.3 | `playback-speed` | Plan 9 - Vitesse | Moyenne |
| C.4 | `picture-in-picture` | Plan 17 - PiP | Moyenne |

---

#### C.1 - Scenario `volume-control`

**Feature testee :** Plan 1 - Bouton Volume

**Flux utilisateur :**
1. L'utilisateur survole le bouton volume
2. Le slider vertical s'ouvre
3. L'utilisateur ajuste le volume
4. L'utilisateur clique pour muter

**Assertions :**
- [ ] Le slider s'ouvre au hover
- [ ] Le slider est vertical (bas = 0, haut = 100)
- [ ] Le clic mute/unmute le son
- [ ] Le slider se ferme apres 1.5s d'inactivite
- [ ] L'icone change selon l'etat (mute, low, high)

**Context :** `authenticated`

---

#### C.2 - Scenario `fullscreen-toggle`

**Feature testee :** Plan 2 - Fullscreen harmonise

**Flux utilisateur :**
1. L'utilisateur clique sur le bouton fullscreen
2. L'application passe en plein ecran
3. Le curseur se cache apres inactivite
4. L'utilisateur appuie sur Echap
5. L'application quitte le fullscreen

**Assertions :**
- [ ] Le bouton fullscreen est visible
- [ ] Double-clic sur la video toggle fullscreen
- [ ] Le curseur se cache apres 2s
- [ ] Echap quitte le fullscreen
- [ ] L'icone change entre enter/exit

**Context :** `authenticated` (note: fullscreen peut etre limite en mode offscreen)

---

#### C.3 - Scenario `playback-speed`

**Feature testee :** Plan 9 - Vitesse lecture intelligente

**Flux utilisateur :**
1. L'utilisateur regarde une VOD cachee (mode replay)
2. L'utilisateur ouvre le selecteur de vitesse
3. L'utilisateur selectionne 1.5x
4. La vitesse change

**Assertions :**
- [ ] Le selecteur de vitesse est disponible en mode replay
- [ ] Le selecteur est desactive/cache en mode live
- [ ] Le changement de vitesse est applique
- [ ] La vitesse se reset a 1x lors d'un nouveau stream

**Context :** `authenticated` + VOD cachee

---

#### C.4 - Scenario `picture-in-picture`

**Feature testee :** Plan 17 - Picture-in-Picture

**Flux utilisateur :**
1. L'utilisateur regarde un stream
2. L'utilisateur clique sur le bouton PiP
3. La fenetre flottante s'ouvre
4. Le placeholder apparait dans la fenetre principale
5. L'utilisateur ferme le PiP

**Assertions :**
- [ ] Le bouton PiP est visible (entre Volume et Fullscreen)
- [ ] Le bouton PiP est desactive en fullscreen
- [ ] Le placeholder "Video en PiP" s'affiche
- [ ] Le raccourci P toggle le PiP
- [ ] Echap ferme le PiP et revient au player

**Context :** `authenticated`

---

### Groupe D : Preferences et Configuration (Priorite Moyenne)

| # | Scenario | Feature testee | Priorite |
|---|----------|----------------|----------|
| D.1 | `preferences-cache` | Plan 12 - Preferences | Moyenne |
| D.2 | `theme-switch` | Plan 18 - Theme | Moyenne |
| D.3 | `language-switch` | Plan 13 - i18n | Moyenne |
| D.4 | `default-quality` | Plan 29 - Qualite defaut | Moyenne |

---

#### D.1 - Scenario `preferences-cache`

**Feature testee :** Plan 12 - Panel Preferences (section cache)

**Flux utilisateur :**
1. L'utilisateur clique sur l'engrenage (preferences)
2. Le panel preferences s'ouvre
3. L'utilisateur voit la section Cache & Stockage
4. L'utilisateur modifie la taille max du cache
5. L'utilisateur ferme le panel

**Assertions :**
- [ ] Le bouton preferences est visible sur la Home
- [ ] Le panel s'ouvre avec le header
- [ ] La section Cache affiche l'espace utilise
- [ ] Le stepper permet de changer la taille max
- [ ] Le bouton retour ferme le panel

**Context :** `authenticated`

---

#### D.2 - Scenario `theme-switch`

**Feature testee :** Plan 18 - Theme Clair/Sombre

**Flux utilisateur :**
1. L'utilisateur ouvre les preferences
2. L'utilisateur voit la section Apparence
3. L'utilisateur change de theme (Auto/Clair/Sombre)
4. L'UI change de couleurs

**Assertions :**
- [ ] Les trois options sont disponibles (Auto, Clair, Sombre)
- [ ] Le changement est applique immediatement
- [ ] Les couleurs de fond et texte changent
- [ ] La preference est sauvegardee

**Context :** `authenticated`

---

#### D.3 - Scenario `language-switch`

**Feature testee :** Plan 13 - Internationalisation

**Flux utilisateur :**
1. L'utilisateur ouvre les preferences
2. L'utilisateur change la langue (EN/FR)
3. Tous les labels changent de langue

**Assertions :**
- [ ] Le dropdown de langue est disponible
- [ ] Le changement est applique a chaud (sans redemarrage)
- [ ] Les sections Home se mettent a jour
- [ ] La preference est sauvegardee

**Context :** `authenticated`

---

#### D.4 - Scenario `default-quality`

**Feature testee :** Plan 29 - Qualite par defaut

**Flux utilisateur :**
1. L'utilisateur configure une qualite par defaut (720p)
2. L'utilisateur ouvre un nouveau stream
3. Le stream demarre en 720p (si disponible)

**Assertions :**
- [ ] Le dropdown qualite par defaut fonctionne
- [ ] La preference est persistee
- [ ] Le nouveau stream utilise la qualite configuree
- [ ] Fallback si qualite non disponible

**Context :** `authenticated` + preference qualite

---

### Groupe E : Navigation et UI (Priorite Basse)

| # | Scenario | Feature testee | Priorite |
|---|----------|----------------|----------|
| E.1 | `navigation-panels` | Plan 28 - Navigation | Basse |
| E.2 | `animations` | Plan 10 - Animations | Basse |

---

#### E.1 - Scenario `navigation-panels`

**Feature testee :** Plan 28 - Strategie Navigation Globale

**Flux utilisateur :**
1. L'utilisateur est sur la Home
2. L'utilisateur ouvre les Preferences
3. Les boutons navigation globaux disparaissent
4. L'utilisateur ferme avec le bouton retour
5. Les boutons reapparaissent

**Assertions :**
- [ ] Les boutons Replays et Preferences sont visibles sur Home
- [ ] Les boutons se cachent quand un panel est ouvert
- [ ] Le PanelHeader est coherent entre les panels
- [ ] La navigation est fluide

**Context :** `authenticated`

---

#### E.2 - Scenario `animations`

**Feature testee :** Plan 10 - Animations & Transitions

**Flux utilisateur :**
1. L'utilisateur navigue entre vues
2. Les transitions sont fluides
3. Les hovers ont des animations

**Assertions :**
- [ ] Les cards ont une animation au hover (scale)
- [ ] Les transitions entre vues sont animees
- [ ] Les popups s'ouvrent avec animation
- [ ] Pas de saccade ou artefact

**Context :** `authenticated`

---

## Dependances du plan

| Dependance | Raison |
|------------|--------|
| **Plan 43 - Refactoring E2E** | Le refactoring simplifie la creation de nouveaux scenarios (BaseE2EContext, E2EScenarioTemplate, helpers enrichis). **Recommande de terminer Plan 43 avant Plan 44.** |

---

## Sous-taches

- **44.1** - Infrastructure fixtures : Creer vods.json, watch_progress.json, irc_messages.json, search_results.json
- **44.2** - Endpoints MockTwitchServer : Ajouter /helix/search, /helix/videos, endpoint VOD
- **44.3** - Context authenticated-with-vods : VOD cachees mockees
- **44.4** - Context authenticated-with-chat : Mock IRC server
- **44.5** - Sprint 1 : Scenarios A.1, A.2, A.3, B.1 (flux critiques)
- **44.6** - Sprint 2 : Scenarios A.4, A.5, C.4 (features differenciantes)
- **44.7** - Sprint 3 : Scenarios C.1-C.3, D.1-D.4 (controles et preferences)
- **44.8** - Sprint 4 : Scenarios B.2, E.1, E.2 (edge cases et polish)

## Priorite des sous-taches

| Priorite | Sous-tache | Dependances | Effort |
|----------|------------|-------------|--------|
| 1 | 44.1 - Fixtures | Plan 43 (recommande) | 0.5 jour |
| 2 | 44.2 - Endpoints | 44.1 | 0.5 jour |
| 3 | 44.3 - Context VODs | 44.1 | 0.5 jour |
| 4 | 44.4 - Context Chat | 44.1 | 0.5 jour |
| 5 | 44.5 - Sprint 1 | 44.2, 44.3 | 2-3 jours |
| 6 | 44.6 - Sprint 2 | 44.5 | 1-2 jours |
| 7 | 44.7 - Sprint 3 | 44.5 | 3-4 jours |
| 8 | 44.8 - Sprint 4 | 44.4, 44.5 | 1-2 jours |

---

## Priorite des scenarios

| Priorite | Scenarios | Raison |
|----------|-----------|--------|
| 1 (Critique) | A.1, A.2, A.3, B.1 | Flux utilisateur principaux |
| 2 (Haute) | A.4, A.5, C.4 | Features differenciantes |
| 3 (Moyenne) | C.1, C.2, C.3, D.1, D.2, D.3, D.4 | Controls et preferences |
| 4 (Basse) | B.2, E.1, E.2 | Edge cases et polish |

---

## Dependencies entre scenarios

| Scenario | Depend de | Raison |
|----------|-----------|--------|
| A.3 (quality-switch) | open-stream existant | Necessite un player actif |
| A.4 (resume-playback) | A.2 (play-cached-vod) | Necessite VOD avec progression |
| A.5 (audio-only) | A.3 (quality-switch) | Utilise le meme selecteur |
| B.2 (chat-messages) | B.1 (chat-open-close) | Necessite chat ouvert |
| C.1, C.2, C.3, C.4 | open-stream existant | Necessitent player actif |
| D.1, D.2, D.3, D.4 | Aucune | Independants |

---

## Fixtures et mocks additionnels requis

| Fixture | Description | Scenarios |
|---------|-------------|-----------|
| `vods.json` | Liste de VOD cachees avec metadonnees | A.2, A.4 |
| `watch_progress.json` | Progression de visionnage sauvegardee | A.4 |
| `irc_messages.json` | Messages chat IRC mockes | B.1, B.2 |
| `multi_quality_playlist.m3u8` | Playlist HLS avec plusieurs qualites | A.3, A.5 |
| `search_results.json` | Resultats de recherche mockes | A.1 |

---

## Contexts a creer

| Context | Description | Scenarios |
|---------|-------------|-----------|
| `authenticated-with-vods` | Auth + VOD cachees mockees | A.2, A.4 |
| `authenticated-with-chat` | Auth + mock IRC server | B.1, B.2 |

---

## Fichiers concernes

### A creer
- `tests/e2e/scenarios/search-stream/`
- `tests/e2e/scenarios/play-cached-vod/`
- `tests/e2e/scenarios/quality-switch/`
- `tests/e2e/scenarios/resume-playback/`
- `tests/e2e/scenarios/audio-only-mode/`
- `tests/e2e/scenarios/chat-open-close/`
- `tests/e2e/scenarios/volume-control/`
- `tests/e2e/scenarios/fullscreen-toggle/`
- `tests/e2e/scenarios/playback-speed/`
- `tests/e2e/scenarios/picture-in-picture/`
- `tests/e2e/scenarios/preferences-cache/`
- `tests/e2e/scenarios/theme-switch/`
- `tests/e2e/scenarios/language-switch/`
- `tests/e2e/scenarios/default-quality/`
- `tests/e2e/scenarios/navigation-panels/`
- `tests/e2e/fixtures/vods.json`
- `tests/e2e/fixtures/watch_progress.json`
- `tests/e2e/fixtures/irc_messages.json`
- `tests/e2e/fixtures/search_results.json`

### A modifier
- `tests/e2e/CMakeLists.txt` - Ajouter les nouveaux scenarios
- `tests/e2e/servers/MockTwitchServer.cpp` - Endpoints recherche, VOD

---

## Checklist de validation

### Infrastructure
- [x] Fixtures JSON creees pour VOD, chat, recherche
- [x] MockTwitchServer supporte les endpoints supplementaires
- [x] MockIrcServer cree pour tests chat
- [x] MockHlsServer supporte VOD et multi-qualite
- [x] Fonction CMake add_e2e_scenario() pour reduire duplication
- [x] Labels e2e/e2e-extended pour profils de test
- [x] Commandes make e2e / make e2e-extended

### Scenarios Priorite 1 (Sprint 1) - COMPLETE
- [x] A.1 - search-stream implementé et passant
- [ ] A.2 - play-cached-vod (reporte Sprint 2)
- [x] A.3 - quality-switch implementé et passant
- [x] B.1 - chat-open-close implementé et passant (avec reception/envoi messages)

### Scenarios Priorite 2 (Sprint 2)
- [ ] A.4 - resume-playback implementé et passant
- [ ] A.5 - audio-only-mode implementé et passant
- [ ] C.4 - picture-in-picture implementé et passant

### Scenarios Priorite 3 (Sprint 3)
- [ ] C.1 - volume-control implementé et passant
- [ ] C.2 - fullscreen-toggle implementé et passant
- [ ] C.3 - playback-speed implementé et passant
- [ ] D.1 - preferences-cache implementé et passant
- [ ] D.2 - theme-switch implementé et passant
- [ ] D.3 - language-switch implementé et passant
- [ ] D.4 - default-quality implementé et passant

### Scenarios Priorite 4 (Sprint 4)
- [ ] B.2 - chat-messages implementé et passant
- [ ] E.1 - navigation-panels implementé et passant
- [ ] E.2 - animations implementé et passant

### Integration CI
- [x] Tous les scenarios s'executent via `make e2e`
- [x] Screenshots captures pour chaque test
- [x] wait() hardcodes remplaces par tryVerify()/clickAndWait()
- [ ] Pas de test flaky apres 3 executions consecutives (parallelisme a resoudre - voir Plan 45)

---

## Estimation

| Sprint | Scenarios | Effort estime |
|--------|-----------|---------------|
| Sprint 1 | A.1, A.2, A.3, B.1 (4) | 2-3 jours |
| Sprint 2 | A.4, A.5, C.4 (3) | 1-2 jours |
| Sprint 3 | C.1-C.3, D.1-D.4 (7) | 3-4 jours |
| Sprint 4 | B.2, E.1, E.2 (3) | 1-2 jours |

**Total : 17 nouveaux scenarios, ~8-11 jours de developpement**

---

## Notes

- Les scenarios utilisent le pattern existant (`E2ETestCase.qml`)
- Chaque scenario est un executable independant
- Le mode offscreen (QT_QPA_PLATFORM=offscreen) peut limiter certains tests (fullscreen)
- Privilegier les assertions sur l'etat UI plutot que le timing

---

## Specifications (Sprint 1)

### Scenarios implementes

| Scenario | Tests | Description |
|----------|-------|-------------|
| `search-stream` | 6 | Recherche + navigation vers player |
| `quality-switch` | 5 | Selecteur qualite, 480p, bouton SD, toast |
| `chat-open-close` | 8 | Toggle chat, connexion IRC, reception/envoi messages |

### Infrastructure creee

- **MockIrcServer** : Serveur WebSocket IRC mock pour tests chat
- **add_e2e_scenario()** : Macro CMake pour reduire duplication (~60 lignes → ~10 lignes par scenario)
- **Labels e2e/e2e-extended** : Profils de test pour CI (rapide) vs nightly (complet)
- **setEchoMessages()** : Echo des messages envoyes pour verification UI

### ObjectNames ajoutes pour E2E

- `QualityControl.qml` : qualityControl, qualityButton, qualityPopup, qualityOption_N, qualityButtonText
- `SearchResults.qml` : searchResultsPopup, searchResult_N
- `ChatPanel.qml` : chatPanel, chatMessageList, chatMessageInput
- `PlayerControlBar.qml` : chatToggleButton
- `PlayerView.qml` : qualityToast

---

## Bonus (au-dela du plan initial)

### Refactorings appliques

| # | Refactoring | Impact |
|---|-------------|--------|
| 1 | Unification namespace MockIrcServer (`E2E` → `blueplayer::test::e2e`) | Consistance |
| 2 | Extraction helper `_traverseTree()` dans E2ETestCase | -40 lignes duplication |
| 3 | Deplacer initTestCase/cleanupTestCase communs dans base | -40 lignes boilerplate |
| 4 | Ajout E2EConstants manquants (search, quality, chat) | Maintenabilite |
| 5 | Standardisation strategie screenshots | Consistance |

### Verifications supplementaires

- **Toast verification** : Apres changement qualite, verification que le toast "Quality: 480p" apparait (opacity > 0, text correct)
- **Message UI verification** : Apres envoi message chat, verification qu'il apparait dans la liste (count augmente)

### Quality Reviews

- `quality/review-03-e2e-plan44-2025-12-29.md` : Code review complet du Sprint 1
