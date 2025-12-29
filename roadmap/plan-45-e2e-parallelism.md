# Plan 45 : Parallelisme Tests E2E

## Contexte

Les tests E2E utilisent des serveurs mock (MockTwitchServer, MockHlsServer, MockIrcServer) sur des ports fixes. Quand plusieurs tests s'executent en parallele (`ctest -j4`), il y a contention sur les ports, provoquant des echecs aleatoires.

## Objectif

Permettre l'execution parallele des tests E2E sans echecs dus a la contention de ports.

## Comportement attendu

### Isolation des ports

Chaque instance de test E2E doit utiliser des ports uniques :
- Un port unique pour MockTwitchServer
- Un port unique pour MockHlsServer  
- Un port unique pour MockIrcServer

### Detection automatique

Le systeme doit detecter automatiquement un port libre au demarrage de chaque serveur mock, au lieu d'utiliser des ports fixes.

### Variables d'environnement

Les URLs des serveurs mock doivent etre passees via variables d'environnement pour que l'application les utilise.

## Checklist de validation

- [ ] Les tests passent avec `ctest -j1` (sequentiel)
- [ ] Les tests passent avec `ctest -j4` (parallele)
- [ ] Aucune erreur "Address already in use"
- [ ] Les logs indiquent les ports utilises par chaque test

## Notes techniques

Solutions possibles :
1. **Port 0** : Laisser le systeme assigner un port libre automatiquement
2. **Port range** : Utiliser une plage de ports basee sur l'ID de test
3. **Retry logic** : Reessayer avec un autre port en cas d'echec

La solution recommandee est le port 0 (allocation dynamique par le systeme).
