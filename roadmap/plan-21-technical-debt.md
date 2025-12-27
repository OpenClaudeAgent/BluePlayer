# Plan 21 - Réduction de la Dette Technique

## Contexte

Après le développement de nombreuses fonctionnalités, il est essentiel de prendre du recul pour :
1. Évaluer la qualité et la maintenabilité de la codebase
2. Identifier et corriger les dettes techniques accumulées
3. Optimiser les performances de l'application

Ce plan est **récurrent** : il devrait être exécuté périodiquement (ex: après chaque milestone majeur).

## Objectifs

### Volet 1 : Audit & Refactoring
- Analyser la structure du projet et l'architecture
- Identifier les code smells et les violations de principes SOLID
- Refactorer les composants problématiques
- Améliorer la maintenabilité pour le futur

### Volet 2 : Performance
- Mesurer les performances actuelles (baseline)
- Identifier les goulots d'étranglement
- Proposer et implémenter des optimisations
- Valider les améliorations avec des benchmarks

---

## Volet 1 : Audit & Refactoring

### 1.1 - Audit de l'architecture

**Questions à répondre :**
- [ ] La séparation des responsabilités est-elle claire ? (UI / Business Logic / Data)
- [ ] Les dépendances entre modules sont-elles saines ? (pas de cycles, couplage faible)
- [ ] Le code est-il facilement testable ?
- [ ] Les abstractions sont-elles au bon niveau ?

**Outils d'analyse :**
```bash
# Analyse statique C++
clang-tidy --checks='*' src/**/*.cpp

# Complexité cyclomatique
lizard src/

# Dépendances (si outil disponible)
# include-what-you-use, cpp-dependencies, etc.
```

### 1.2 - Revue de la structure des fichiers

**Structure actuelle à auditer :**
```
src/
├── api/twitch/        # API Twitch
├── chat/              # Chat IRC
├── core/              # Logique métier + utils
│   └── network/       # HTTP, cache réseau
├── media/             # mpv, HLS
└── ui/                # QML + ViewModels
    ├── components/    # Composants réutilisables
    └── themes/        # Thèmes visuels
```

**Questions :**
- [ ] Le dossier `core/` est-il devenu un fourre-tout ?
- [ ] Faut-il extraire certains modules ? (ex: `core/cache/`, `core/auth/`)
- [ ] Les ViewModels sont-ils au bon endroit ?
- [ ] La séparation C++ / QML est-elle cohérente ?

### 1.3 - Code smells à rechercher

| Smell | Description | Fichiers suspects |
|-------|-------------|-------------------|
| God Class | Classe qui fait trop de choses | `TwitchService`, `MpvQuickItem` ? |
| Long Method | Méthodes > 50 lignes | À identifier |
| Duplicate Code | Code copié-collé | Entre les cards QML ? |
| Magic Numbers | Constantes hardcodées | Timeouts, tailles, etc. |
| Dead Code | Code non utilisé | Fonctions obsolètes |
| Inconsistent Naming | Nommage incohérent | À vérifier |

### 1.4 - Checklist refactoring

**C++ :**
- [ ] Extraire les constantes dans `Constants.hpp`
- [ ] Uniformiser la gestion d'erreurs (`Result<T>` partout ?)
- [ ] Vérifier les includes (forward declarations quand possible)
- [ ] Documenter les classes publiques (Doxygen)
- [ ] Supprimer le code mort

**QML :**
- [ ] Factoriser les styles dupliqués dans AppleTheme.js
- [ ] Extraire les composants réutilisables
- [ ] Uniformiser les bindings et signaux
- [ ] Vérifier les memory leaks (Connections, Timers)
- [ ] Simplifier les états complexes (StateGroup)

---

## Volet 2 : Analyse de Performance

### 2.1 - Métriques à mesurer

| Métrique | Outil | Cible |
|----------|-------|-------|
| Temps de démarrage | `time` + logs | < 2s |
| Utilisation CPU (idle) | Activity Monitor / `top` | < 5% |
| Utilisation CPU (lecture) | Activity Monitor | < 30% |
| Utilisation RAM | Activity Monitor | < 300MB |
| Frame drops | mpv stats | 0 |
| Latence UI | Instruments (Time Profiler) | < 16ms/frame |

### 2.2 - Outils de profiling macOS

**Instruments (Xcode) :**
```bash
# Time Profiler - CPU usage
instruments -t "Time Profiler" -D output.trace ./BluePlayer

# Allocations - Memory
instruments -t "Allocations" -D output.trace ./BluePlayer

# Leaks - Memory leaks
instruments -t "Leaks" -D output.trace ./BluePlayer
```

**Sampling manuel :**
```bash
# Sample process pendant 10 secondes
sample BluePlayer 10 -file sample_output.txt
```

**mpv stats :**
- Appuyer sur `i` pendant la lecture pour afficher les stats
- Frame drops, A/V sync, cache status

### 2.3 - Points chauds potentiels

| Zone | Risque | Vérification |
|------|--------|--------------|
| Chat IRC (plan 7) | Messages fréquents → UI lag | Throttling, virtualisation |
| Thumbnails | Chargement simultané → RAM | Cache LRU, lazy loading |
| Animations QML | Trop d'animations → GPU | Réduire durées, simplifier |
| Network requests | Requêtes bloquantes | Vérifier async |
| JSON parsing | Gros payloads | Streaming parser ? |
| HLS/mpv | Décodage vidéo | Hardware acceleration |

### 2.4 - Optimisations courantes

**C++ :**
- [ ] Move semantics utilisé correctement
- [ ] Éviter les copies inutiles (const ref)
- [ ] String views au lieu de copies
- [ ] Réutiliser les buffers
- [ ] Lazy initialization

**QML :**
- [ ] `visible: false` pour les éléments cachés (pas juste opacity: 0)
- [ ] Loader pour les composants lourds
- [ ] Image.asynchronous: true
- [ ] ListView.cacheBuffer optimisé
- [ ] Éviter les bindings complexes

**Réseau :**
- [ ] Connection pooling (HTTP/2)
- [ ] Cache agressif (images, API responses)
- [ ] Compression des requêtes
- [ ] Prefetching intelligent

---

## Processus d'exécution

### Étape 1 : Baseline
1. Documenter les métriques actuelles
2. Créer un rapport de référence
3. Identifier les 3 problèmes les plus critiques

### Étape 2 : Audit
1. Exécuter les outils d'analyse statique
2. Revue manuelle de l'architecture
3. Lister les dettes techniques

### Étape 3 : Priorisation
1. Classer par impact (High/Medium/Low)
2. Classer par effort (S/M/L/XL)
3. Choisir les quick wins (High impact + Low effort)

### Étape 4 : Refactoring
1. Créer une branche dédiée
2. Implémenter les corrections par petits commits
3. Tester chaque changement
4. Mesurer l'impact

### Étape 5 : Validation
1. Comparer les métriques avant/après
2. Vérifier pas de régression fonctionnelle
3. Documenter les améliorations

---

## Livrables attendus

1. **Rapport d'audit** : État de la codebase, dettes identifiées
2. **Rapport de performance** : Baseline + points chauds
3. **Liste priorisée** : Corrections à effectuer
4. **Refactoring** : Commits de nettoyage
5. **Métriques finales** : Comparaison avant/après

---

## Fichiers concernés

**Potentiellement tous**, mais focus sur :
- `src/api/twitch/TwitchService.cpp` - Potentielle God Class
- `src/media/MpvQuickItem.cpp` - Complexité élevée
- `src/core/` - Structure à revoir
- `src/ui/components/*.qml` - Duplication potentielle

---

## Checklist de validation

### Audit
- [ ] Analyse statique exécutée (clang-tidy)
- [ ] Complexité mesurée (lizard ou équivalent)
- [ ] Structure des fichiers revue
- [ ] Code smells documentés
- [ ] Dette technique listée et priorisée

### Refactoring
- [ ] Constantes extraites
- [ ] Code mort supprimé
- [ ] Duplication réduite
- [ ] Nommage uniformisé
- [ ] Documentation ajoutée

### Performance
- [ ] Baseline documentée (CPU, RAM, temps démarrage)
- [ ] Profiling effectué (Instruments)
- [ ] Goulots d'étranglement identifiés
- [ ] Optimisations implémentées
- [ ] Métriques améliorées (mesures avant/après)

### Qualité
- [ ] Tous les tests passent
- [ ] Pas de régression fonctionnelle
- [ ] Build propre (pas de warnings)
- [ ] Code review effectuée

---

## Notes

Ce plan est conçu pour être **exécuté périodiquement**, idéalement :
- Après chaque milestone majeur (ex: v1.0, v2.0)
- Quand la vélocité de développement diminue
- Avant une phase de features critiques

**Recommandation** : Exécuter ce plan après avoir terminé les plans 7-13 (avant d'attaquer 14-20).
