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

Transformer les idees et besoins en plans d'implementation structures, clairs et actionables.

### Responsabilites

- Analyser les besoins exprimes par l'utilisateur
- Structurer les fonctionnalites en plans detailles
- Definir les criteres d'acceptance
- Maintenir la vision globale du projet

### Contraintes

- Ne modifie jamais le code source
- Travaille exclusivement dans le dossier `roadmap/`
- Les plans sont immutables une fois crees
- Seul le suivi de statut peut etre mis a jour

### Artefacts produits

| Artefact | Description | Mutabilite |
|----------|-------------|------------|
| `plan-XX-*.md` | Plan detaille d'une fonctionnalite | Immutable |
| `README.md` | Suivi global et methodologie | Statut uniquement |

### Workflow

```
[Ideation] --> [Clarification] --> [Redaction du plan] --> [Validation] --> [Publication]
     ^              |                      |                    |               |
     |              v                      v                    v               v
   Besoin      Questions            Specifications         Criteres        Plan
   utilisateur a l'utilisateur      techniques            d'acceptance    immutable
```

1. **Ideation** : L'utilisateur exprime un besoin ou une idee
2. **Clarification** : L'agent pose des questions pour comprendre le contexte
3. **Redaction** : Creation du plan avec specifications et criteres
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
[Selection] --> [Preparation] --> [Implementation] --> [Tests?] --> [Validation] --> [Finalisation]
     |               |                  |                 |               |                |
     v               v                  v                 v               v                v
  Prochaine     Branche Git         Code +          Si echec:       Checklist        Commit +
  tache         + sync main         Build OK        Tester->Quality utilisateur      Proposition merge
```

1. **Selection** : Identification de la prochaine tache selon priorites et dependances
2. **Preparation** : Synchronisation avec main et creation de la branche feature
3. **Implementation** : Developpement dans `src/` selon les specifications du plan
4. **Tests** : Si des tests echouent, invoquer Tester puis Quality pour validation
5. **Validation** : Presentation de la checklist, iterations jusqu'a validation complete
6. **Finalisation** : Commit, mise a jour des statuts, proposition de merge

### Flux Executeur-Tester-Quality

Quand des tests echouent ou sont necessaires :

```
+----------+     (1) Tests      +----------+     (3) Validation    +----------+
|Executeur |---->  echouent --->|  Tester  |---->  demandee  ----->| Quality  |
+----------+                    +----------+                       +----------+
     ^                               |                                  |
     |                               v                                  v
     |                          (2) Tests                          (4) Rapport
     |                           repares                            validation
     |                               |                                  |
     +-------------------------------+----------------------------------+
                                     |
                                     v
                              (5) Continue si OK
                              ou resout problemes
```

### Cycle de validation

```
                 +------------------+
                 |  Presentation    |
                 |  checklist       |
                 +--------+---------+
                          |
                          v
                 +--------+---------+
                 |  Utilisateur     |
                 |  teste           |
                 +--------+---------+
                          |
              +-----------+-----------+
              |                       |
              v                       v
     +--------+--------+     +--------+--------+
     |   Probleme      |     |   Tout OK       |
     |   detecte       |     |                 |
     +--------+--------+     +--------+--------+
              |                       |
              v                       v
     +--------+--------+     +--------+--------+
     |   Correction    |     |   Finalisation  |
     +--------+--------+     +-----------------+
              |
              +-------> (retour presentation)
```

---

## Agent Quality

### Mission

Definir la strategie de test globale du produit en consolidant les criteres de validation et en identifiant les regressions potentielles. L'agent Quality ne realise pas les tests lui-meme : il produit les plans et scenarios de test que l'utilisateur executera.

### Responsabilites

- Consolider toutes les checklists de validation des plans termines
- Identifier les criteres obsoletes ou modifies
- Detecter les regressions potentielles entre fonctionnalites
- Produire des plans de tests manuels structures
- Definir des scenarios de test clairs et actionables
- Accompagner l'utilisateur pendant l'execution des tests
- **Valider les changements de tests** demandes par l'Executeur apres intervention du Tester
- **Maintenir l'historique** des analyses et decisions pour ameliorer les futures evaluations

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
|   5. MERGE        |
|   (Utilisateur)   |
+---------+---------+
          |
          v
+---------+---------+       +---------+---------+
|   6. QUALITE      |       |   6b. TESTS       |
|   (Quality)       |       | (Tester+Refactor) |
+---------+---------+       +---------+---------+
          |                           |
          +-------------+-------------+
                        |
                        v
              +---------+---------+
              |   7. RELEASE      |
              |   (Utilisateur)   |
              +-------------------+
```

**Note** : Les etapes 6 (Qualite) et 6b (Tests) peuvent etre executees en parallele. Quality produit des plans de tests manuels, Tester produit des tests automatises.

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
| Principal | main / feature/* | Utilisateur | Lecture/Ecriture |
| Feature | worktree/feature | Executeur | Lecture/Ecriture |
| Roadmap | worktree/roadmap | Roadmap | Lecture/Ecriture |
| Quality | worktree/quality | Quality | Lecture/Ecriture |
| Test | worktree/test | Tester | Lecture/Ecriture |
| Refactoring | worktree/refactoring | Refactoring | Lecture/Ecriture |

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
