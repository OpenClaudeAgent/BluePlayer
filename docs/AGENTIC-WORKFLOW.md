# Agentic Workflow

Ce repository est developpe selon une methodologie de developpement assistee par agents IA (Claude Opus via OpenCode). Ce document decrit l'architecture des agents, leurs responsabilites, et les workflows qui orchestrent le developpement du projet.

## Vue d'ensemble

Le developpement est structure autour de cinq agents specialises, chacun operant dans un environnement isole (worktree Git) pour permettre le travail parallele sans conflits.

```
                                    +------------------+
                                    |   Utilisateur    |
                                    +--------+---------+
                                             |
         +------------+------------+---------+---------+------------+
         |            |            |                   |            |
         v            v            v                   v            v
    +----+----+  +----+----+  +----+----+        +----+----+  +----+----+
    | Roadmap |  |Executeur|  | Quality |        | Tester  |  |Refactor |
    |(Planif.)|  |(Implem.)|  |(Strat.) |        |(Tests)  |  |(Testab.)|
    +----+----+  +----+----+  +----+----+        +----+----+  +----+----+
         |            |            |                   |            |
         v            |            v                   |            |
    +----+----+       |       +----+----+              +-----+------+
    |roadmap/ |       |       |quality/ |                    |
    |- Plans  |------>|<------|- Strat. |                    |
    |- Specs  |       |       |- Scenar.|                    |
    +---------+       |       +---------+                    |
                      v                                      v
                 +----+----+                           +----+----+
                 |  Code   |<--------------------------|  Code   |
                 |  src/   |    (testabilite)          | tests/  |
                 +---------+                           +---------+

    Legende:
    - Roadmap, Executeur, Quality : Cycle feature (planification -> implementation -> validation)
    - Tester, Refactoring : Cycle qualite technique (tests -> refactoring -> tests)
```

## Principes fondamentaux

### Isolation des agents

Chaque agent opere dans son propre espace de travail (worktree Git), ce qui garantit :

- **Pas de conflits** : Les agents peuvent travailler simultanement sans se bloquer
- **Tracabilite** : Chaque contribution est isolee sur sa propre branche
- **Controle utilisateur** : La branche principale reste sous controle exclusif de l'utilisateur

### Separation des responsabilites

| Agent | Responsabilite | Produit | Consomme |
|-------|---------------|---------|----------|
| Roadmap | Planification | Plans, specifications | - |
| Executeur | Implementation | Code source (`src/`) | Plans |
| Quality | Strategie QA + Validation | Plans de test, validations, historique | Plans, tests |
| Tester | Tests automatises | Tests unitaires, integration, coverage | Code |
| Refactoring | Testabilite | Code refactorise, interfaces | Demandes du Tester |

### Communication inter-agents

Les agents ne communiquent jamais directement entre eux. Toute communication passe par :

1. **Les artefacts** : Documents ecrits dans les dossiers dedies
2. **L'utilisateur** : Qui orchestre et valide les transitions

**Exceptions** :
- Le tandem Tester-Refactoring peut collaborer directement (avec validation utilisateur)
- L'Executeur peut invoquer Tester puis Quality pour validation des tests

### Regles globales

Ces regles s'appliquent a TOUS les agents :

| Regle | Description |
|-------|-------------|
| **Dates systeme** | Toujours utiliser `date +%Y-%m-%d` pour obtenir la date - ne jamais la deviner |
| **Worktrees** | Chaque agent travaille dans son worktree dedie |
| **Validation utilisateur** | Aucun merge sur main sans approbation explicite |
| **Isolation** | Ne pas modifier les fichiers hors de son scope |

---

## Agent Roadmap

### Mission

Transformer les idees et besoins en plans fonctionnels structures, clairs et actionables.

### Responsabilites

- Analyser les besoins exprimes par l'utilisateur
- Structurer les fonctionnalites en plans detailles
- Decrire les **comportements attendus** (ce que l'utilisateur voit et fait)
- Definir les criteres d'acceptance
- Maintenir la vision globale du projet

### Contraintes

- Ne modifie jamais le code source
- Travaille exclusivement dans le dossier `roadmap/`
- **L'idee est immutable** : Contexte, Objectif, Comportement attendu ne changent pas
- **Specifications et Checklist sont mutables** : Peuvent etre enrichies par l'Executeur
- **Specifications fonctionnelles** : pas de code, quelques mentions techniques OK
- Decrit le **QUOI** (comportement), pas le **COMMENT** (implementation)

### Artefacts produits

| Artefact | Description | Mutabilite |
|----------|-------------|------------|
| `plan-XX-*.md` | Plan fonctionnel | Idee immutable, Specs/Checklist mutables |
| `README.md` | Suivi global et methodologie | Statut uniquement |

### Structure d'un plan

```markdown
# Plan XX - [Titre]

## Contexte                         ← IMMUTABLE
## Objectif                         ← IMMUTABLE
## Comportement attendu             ← IMMUTABLE

## Specifications                   ← MUTABLE (enrichi par Executeur)
## Checklist de validation          ← MUTABLE
```

### Workflow

```
[Ideation] --> [Clarification] --> [Redaction du plan] --> [Validation] --> [Publication]
     ^              |                      |                    |               |
     |              v                      v                    v               v
   Besoin      Questions            Comportements          Criteres        Plan
   utilisateur a l'utilisateur      attendus (UX)         d'acceptance    immutable
```

1. **Ideation** : L'utilisateur exprime un besoin ou une idee
2. **Clarification** : L'agent pose des questions pour comprendre le contexte
3. **Redaction** : Creation du plan avec comportements attendus et criteres
4. **Validation** : L'utilisateur valide la structure du plan
5. **Publication** : Le plan est cree et devient immutable

---

## Agent Executeur

### Mission

Implementer les fonctionnalites selon les plans definis, en garantissant la qualite technique et la validation utilisateur.

### Responsabilites

- Lire et comprendre les plans de la roadmap
- Implementer les specifications techniques dans `src/`
- Garantir un code fonctionnel (build sans erreurs)
- Invoquer l'agent Tester si des tests echouent ou sont necessaires
- Demander validation a l'agent Quality apres intervention du Tester
- Guider l'utilisateur dans la validation
- Documenter les bonus et ajouts

### Contraintes

- Suit strictement les specifications du plan
- **Ne modifie JAMAIS le dossier `tests/`** - delegue au Tester
- Ne modifie pas la structure des plans (sauf checkboxes)
- Attend toujours la validation explicite de l'utilisateur
- Ne merge jamais sur main sans autorisation

### Artefacts produits

| Artefact | Description |
|----------|-------------|
| Code source | Implementation dans `src/` uniquement |
| Checkboxes | Validation des criteres dans le plan |
| Changelog | Mise a jour du changelog principal |

### Workflow

```
[Selection] --> [Preparation] --> [Implementation] --> [Validation] --> [Tests] --> [Quality] --> [Finalisation]
     |               |                  |                   |               |            |              |
     v               v                  v                   v               v            v              v
  Prochaine     Worktree          Code +              Scenarios       Tester      Code Review      Commit +
  tache         par feature       Build OK            utilisateur                 + Tests Review   Merge
```

1. **Selection** : Identification de la prochaine tache selon priorites et dependances
2. **Preparation** : Creation d'un worktree dedie pour la feature (`git worktree add worktrees/feature/[nom] -b feature/[nom]`)
3. **Implementation** : Developpement dans `src/` selon les specifications. Si changements importants, invoquer l'agent Refactoring.
4. **Validation** : Presentation des scenarios de test avec actions concretes, notification via MCP `ask_user`, iterations jusqu'a validation complete
5. **Tests** : Invoquer l'agent Tester pour ecrire les tests automatises
6. **Quality** : Invoquer l'agent Quality pour code review + tests review
7. **Finalisation** : Commit, mise a jour des statuts, proposition de merge via MCP `ask_user`

### Flux Executeur-Tester-Quality

Apres validation utilisateur, l'Executeur invoque systematiquement Tester puis Quality :

```
+----------+     (1) Validation   +----------+     (2) Invoque     +----------+
|Executeur |---->  OK utilisateur |Executeur |---->  Tester  ----->|  Tester  |
+----------+                      +----------+                     +----------+
                                       |                                |
                                       |                           (3) Tests
                                       |                            ecrits
                                       |                                |
                                       v                                v
                                  +----------+     (4) Invoque     +----------+
                                  |Executeur |---->  Quality  ---->| Quality  |
                                  +----------+                     +----------+
                                       ^                                |
                                       |                           (5) Code Review
                                       |                            + Tests Review
                                       |                                |
                                       +--------------------------------+
                                                     |
                                                     v
                                              (6) Rapport OK?
                                              Si oui → Merge
```

### Cycle de validation

L'Executeur genere des **scenarios de test avec actions concretes** :

```markdown
## Validation - [Nom de la tache]

### Scenario 1 : [Comportement principal]
1. [Action concrete : "Clique sur X" / "Ouvre le menu Y"]
2. [Action concrete : "Saisis Z dans le champ"]
3. **Attendu** : [Resultat visible attendu]

### Scenario 2 : [Edge case]
1. [Action concrete]
2. **Attendu** : [Comportement attendu]
```

```
                 +------------------+
                 |  Presentation    |
                 |  scenarios       |
                 +--------+---------+
                          |
                          v
                 +--------+---------+
                 | 🔔 MCP ask_user  |
                 | "Validation"     |
                 +--------+---------+
                          |
              +-----------+-----------+
              |                       |
              v                       v
     +--------+--------+     +--------+--------+
     |   Probleme      |     |   Tout OK       |
     +--------+--------+     +--------+--------+
              |                       |
              v                       v
     +--------+--------+     +--------+--------+
     |   Correction    |     | Tests + Quality |
     +--------+--------+     +-----------------+
              |
              +-------> (retour presentation)
```

---

## Agent Quality

### Mission

Garantir la qualite globale du projet via le code review, la validation des tests, et la production de plans de tests manuels.

### Responsabilites

- **Code Review** : Analyser le code source (src/) avec les principes de clean code
- **Tests Review** : Valider les changements de tests effectues par le Tester
- Consolider toutes les checklists de validation des plans termines
- Identifier les criteres obsoletes ou modifies
- Detecter les regressions potentielles entre fonctionnalites
- Produire des plans de tests manuels structures
- Accompagner l'utilisateur pendant l'execution des tests
- **Maintenir l'historique** des analyses et decisions

### Double Review (invoque par Executeur)

```
Executeur invoque Quality
       ↓
Phase 1: CODE REVIEW (src/)
       ↓
Phase 2: TESTS REVIEW (tests/)
       ↓
Rapport consolide → Executeur
```

### Contraintes

- Ne modifie jamais le code source
- Ne modifie jamais la roadmap (lecture seule)
- Travaille exclusivement dans le dossier `quality/`
- Ne cree pas de tickets ou d'issues
- Ne realise pas les tests (role de l'utilisateur)
- Accede au worktree test (`worktrees/test/`) en lecture pour valider les changements

### Artefacts produits

| Artefact | Description | Mutabilite |
|----------|-------------|------------|
| `report-XX-*.md` | Plan de test avec scenarios | Mutable (resultats) |
| `validation-XX-*.md` | Validation des changements de tests | Mutable |
| `HISTORY.md` | Historique des analyses et decisions | Mutable |
| `README.md` | Methodologie | Immutable |
| `STATUS.md` | Suivi des rapports | Mutable |

### Workflow

```
[Analyse] --> [Phase 1: Consolidation] --> [Phase 2: Impacts] --> [Redaction] --> [Accompagnement]
    |                  |                          |                    |                |
    v                  v                          v                    v                v
 Lecture          Extraction              Identification          Plan de         Guidage
 roadmap          checklists              regressions             test            utilisateur
```

### Phase 1 : Consolidation des checklists

```
+------------------+     +------------------+     +------------------+
|   Plan-01        |     |   Plan-02        |     |   Plan-XX        |
|   Checklist      |     |   Checklist      |     |   Checklist      |
+--------+---------+     +--------+---------+     +--------+---------+
         |                        |                        |
         +------------------------+------------------------+
                                  |
                                  v
                    +-------------+-------------+
                    |   Extraction de tous     |
                    |   les criteres           |
                    +-------------+-------------+
                                  |
                                  v
                    +-------------+-------------+
                    |   Verification           |
                    |   d'obsolescence         |
                    +-------------+-------------+
                                  |
                    +-------------+-------------+
                    |             |             |
                    v             v             v
               +--------+   +--------+   +--------+
               | Valide |   | Modifie|   |Obsolete|
               +--------+   +--------+   +--------+
                    |             |
                    v             v
                    +-------------+
                          |
                          v
              +-----------+-----------+
              |   Liste consolidee    |
              |   des checks valides  |
              +-----------------------+
```

### Phase 2 : Analyse des impacts

```
+------------------+     +------------------+
|   Features       |     |   Composants     |
|   recentes       |     |   partages       |
+--------+---------+     +--------+---------+
         |                        |
         +------------------------+
                    |
                    v
          +---------+---------+
          |   Matrice         |
          |   d'impact        |
          +---------+---------+
                    |
                    v
          +---------+---------+
          |   Checks de       |
          |   regression      |
          |   supplementaires |
          +-------------------+
```

---

## Agent Tester

### Mission

Garantir la qualite du code a travers une strategie de test complete et rigoureuse. Ameliorer la couverture, la qualite et la maintenabilite des tests automatises.

### Responsabilites

- Analyser la couverture de code existante
- Identifier les fichiers et branches non testes
- Ecrire des tests unitaires, d'integration et E2E
- Eliminer les tests flaky (non deterministes)
- Refactoriser les tests pour ameliorer la maintenabilite
- Collaborer avec l'agent Refactoring quand le code n'est pas testable

### Contraintes

- Travaille exclusivement dans le worktree test
- Zero tolerance pour les tests flaky
- Les tests doivent respecter les memes standards que le code production
- Demande toujours l'autorisation avant d'invoquer l'agent Refactoring

### Dimensions de la qualite des tests

| Dimension | Description | Standard |
|-----------|-------------|----------|
| Coverage | Proportion du code executee par les tests | Minimum 70%, cible 85% |
| Qualite | Structure AAA, pas de logique dans les tests | Un test = une assertion |
| Maintenabilite | Tests faciles a comprendre et modifier | DRY sans sacrifier la clarte |
| Determinisme | Tests reproductibles | Zero tests flaky |

### Pyramide des tests

```
        /\
       /  \      E2E (5%)
      /----\     
     /      \    Integration (15%)
    /--------\   
   /          \  Unit (80%)
  --------------
```

### Workflow

```
[Inventaire] --> [Analyse] --> [Implementation] --> [Verification] --> [Integration]
     |              |                |                    |                 |
     v              v                v                    v                 v
  Fichiers     Identification    Ecriture des      Tests passent      Merge sur
  sans tests   des lacunes       nouveaux tests    et deterministes   main
```

### Collaboration avec Refactoring

Quand le code n'est pas testable, le Tester peut demander l'aide du Refactoring :

```
+----------+     (1) Demande         +-------------+
|  Tester  |------------------------>| Utilisateur |
+----------+     autorisation        +------+------+
                                            |
                                     (2) Validation
                                            |
                                            v
                                    +-------+-------+
                                    |  Refactoring  |
                                    +-------+-------+
                                            |
                                     (3) Commit dans
                                         worktree/refactoring
                                            |
+----------+     (4) Cherry-pick     +------+------+
|  Tester  |<------------------------|   Commit    |
+----------+     ou patch            +-------------+
```

---

## Agent Refactoring

### Mission

Ameliorer la testabilite et la maintenabilite du code via des patterns reconnus. Eliminer les anti-patterns qui empechent l'ecriture de tests efficaces.

### Responsabilites

- Appliquer les principes SOLID
- Introduire l'injection de dependances
- Extraire des interfaces pour permettre le mocking
- Eliminer l'etat global et les singletons problematiques
- Supprimer les effets de bord dans les constructeurs
- Documenter les changements architecturaux

### Contraintes

- Travaille exclusivement dans le worktree refactoring
- Ne casse jamais la compilation
- Preserve la retro-compatibilite
- Fait des commits incrementaux avec messages clairs
- Ne merge pas sur main (laisse l'utilisateur decider)

### Anti-patterns traites

| Anti-pattern | Probleme | Resolution |
|--------------|----------|------------|
| Dependances hard-codees | Impossible de mocker | Injection de dependances |
| Etat global / Singletons | Tests s'influencent | Instance injectable |
| Effets de bord constructeur | Tests declenchent I/O | Methode init() separee |
| Variables d'env directes | Tests dependent de l'env | Objet de configuration |
| God Object | Trop de dependances | Extraction de classes |
| Law of Demeter violations | Chaines d'appels | Tell, Don't Ask |

### Principes SOLID appliques

- **S**ingle Responsibility : Une classe = une raison de changer
- **O**pen/Closed : Ouvert a l'extension, ferme a la modification
- **L**iskov Substitution : Les sous-types doivent etre substituables
- **I**nterface Segregation : Interfaces specifiques plutot que generales
- **D**ependency Inversion : Dependre des abstractions, pas des implementations

### Workflow

```
[Identification] --> [Analyse] --> [Planification] --> [Refactoring] --> [Commit]
       |                |                |                  |               |
       v                v                v                  v               v
   Anti-pattern     Impact et       Etapes           Changement       Message
   detecte          dependances     incrementales    minimal          explicatif
```

### Communication du resultat

Apres chaque refactoring, l'agent communique :
- La branche utilisee
- Le hash du commit cree
- Un resume des changements
- Les instructions pour integrer dans un autre worktree

---

## Tandem Tester-Refactoring

### Principe

Les agents Tester et Refactoring forment un tandem complementaire :

```
+----------+                              +-------------+
|  Tester  |                              | Refactoring |
+----+-----+                              +------+------+
     |                                           |
     | Identifie code                            |
     | non testable                              |
     v                                           |
+----+-----+                                     |
|Utilisateur|-----(validation)------------------>|
+----+-----+                                     |
     |                                           v
     |                                    +------+------+
     |                                    | Refactore   |
     |                                    | pour        |
     |                                    | testabilite |
     |                                    +------+------+
     |                                           |
     |<----------(commit a integrer)-------------+
     |
     v
+----+-----+
| Ecrit    |
| les tests|
+----------+
```

### Regles de collaboration

1. **Isolation** : Chaque agent travaille dans son propre worktree
2. **Validation** : L'utilisateur approuve chaque invocation inter-agents
3. **Pas de merge direct** : Seul l'utilisateur merge sur main
4. **Communication par commits** : Cherry-pick ou patches pour transferer le travail

---

## Orchestration globale

### Cycle de vie d'une fonctionnalite

```
+-------------------+
|   1. IDEATION     |
|   (Utilisateur)   |
+---------+---------+
          |
          v
+---------+---------+
|   2. PLANIFICATION|
|   (Roadmap)       |
+---------+---------+
          |
          v
+---------+---------+
|   3. IMPLEMENTATION|
|   (Executeur)     |
+---------+---------+
          |
          v
+---------+---------+
|   4. VALIDATION   |
|   (Utilisateur)   |
+---------+---------+
          |
          v
+---------+---------+
|   5. TESTS        |
|   (Tester)        |
+---------+---------+
          |
          v
+---------+---------+
|   6. QUALITY      |
|   (Code Review +  |
|    Tests Review)  |
+---------+---------+
          |
          v
+---------+---------+
|   7. MERGE        |
|   (Utilisateur)   |
+---------+---------+
          |
          v
+---------+---------+
|   8. RELEASE      |
|   (Utilisateur)   |
+-------------------+
```

**Important** : Les etapes Tests (5) et Quality (6) sont executees **AVANT** le merge, pas apres. On valide d'abord manuellement, puis on ecrit les tests automatises et on fait le code review.

### Matrice des interactions

| Source | Destination | Type | Contenu |
|--------|-------------|------|---------|
| Utilisateur | Roadmap | Demande | Idees, besoins |
| Roadmap | roadmap/ | Production | Plans, specs |
| Quality | roadmap/ | Lecture | Plans pour analyse |
| Quality | quality/ | Production | Plans de test manuels, scenarios |
| Executeur | roadmap/ | Lecture | Plans a implementer |
| Executeur | quality/ | Lecture | Criteres de validation |
| Executeur | src/ | Production | Implementation |
| Tester | src/ | Lecture | Code source a tester |
| Tester | tests/ | Production | Tests automatises |
| Tester | Refactoring | Demande | Code non testable |
| Refactoring | src/ | Production | Code refactorise (testabilite) |
| Refactoring | Tester | Reponse | Commits a integrer |
| Utilisateur | Tous | Validation | Approbations |

### Flux de donnees

```
                              UTILISATEUR
                                   |
         +------------+------------+------------+------------+
         |            |            |            |            |
         v            v            v            v            v
    +---------+  +---------+  +---------+  +---------+  +-----------+
    | Roadmap |  |Executeur|  | Quality |  | Tester  |  |Refactoring|
    +---------+  +---------+  +---------+  +---------+  +-----------+
         |            ^            |            |            |
         v            |            v            v            v
    +---------+       |       +---------+  +---------+  +---------+
    |roadmap/ |-------+       |quality/ |  | tests/  |  |  src/   |
    +---------+       |       +---------+  +---------+  |(testab.)|
                      |                         |       +---------+
                      v                         |            |
                 +---------+                    |            |
                 |  src/   |<-------------------+------------+
                 +---------+
                      |
                      v
                 +---------+
                 |  main   |
                 +---------+

    Legende:
    - Roadmap ecrit dans roadmap/
    - Quality ecrit dans quality/
    - Executeur lit roadmap/ et quality/, ecrit dans src/
    - Tester ecrit dans tests/
    - Refactoring ameliore src/ pour la testabilite
```

---

## Isolation technique

### Environnements de travail

| Environnement | Branche | Agent | Acces |
|---------------|---------|-------|-------|
| Principal | main | Utilisateur | Lecture/Ecriture |
| Feature | `worktrees/feature/[nom]` | Executeur (1 par feature) | Lecture/Ecriture |
| Roadmap | worktree/roadmap | Roadmap | Lecture/Ecriture |
| Quality | worktree/quality | Quality | Lecture/Ecriture |
| Test | worktree/test | Tester | Lecture/Ecriture |
| Refactoring | worktree/refactoring | Refactoring | Lecture/Ecriture |

**Executeur** cree un worktree dedie pour chaque feature :
```bash
git worktree add worktrees/feature/[nom] -b feature/[nom]
```

### Synchronisation

Chaque environnement isole se synchronise regulierement avec la branche principale pour garantir la coherence des donnees.

---

## Avantages de cette approche

### Pour le developpement

- **Parallelisation** : Plusieurs agents peuvent travailler simultanement
- **Tracabilite** : Chaque contribution est isolee et identifiable
- **Reversibilite** : Possibilite de revenir en arriere facilement
- **Qualite** : Processus de validation structure

### Pour la gestion de projet

- **Visibilite** : Etat du projet toujours visible via la roadmap
- **Documentation** : Plans et rapports auto-documentes
- **Predictibilite** : Workflow reproductible et coherent

### Pour l'utilisateur

- **Controle** : Validation explicite a chaque etape cle
- **Flexibilite** : Possibilite d'intervenir a tout moment
- **Transparence** : Comprehension claire de ce que fait chaque agent
