# Plan 46 - Securisation des Credentials et Certificats

## Contexte

Les fichiers suivants ont ete accidentellement commites sur GitHub et restent dans l'historique git :

| Fichier | Contenu sensible |
|---------|------------------|
| `.env` | `TWITCH_CLIENT_ID`, `TWITCH_CLIENT_SECRET` |
| `certs/twitch-cert.pem` | Certificat TLS pour OAuth localhost |
| `certs/twitch-key.pem` | Cle privee TLS pour OAuth localhost |

Ces fichiers ont depuis ete retires du tracking git (via `.gitignore`), mais **restent visibles dans l'historique** des commits. Cela represente un risque de securite car :

1. **Le Client Secret Twitch est compromis** - Toute personne ayant acces au repo peut le recuperer
2. **Les certificats TLS sont exposes** - Bien que locaux, ils pourraient etre reutilises
3. **Aucune documentation** - Les nouveaux developpeurs ne savent pas comment configurer l'environnement

## Objectif

1. **Neutraliser la fuite** - Revoquer les credentials compromis et en generer de nouveaux
2. **Automatiser le setup** - Creer un script qui configure l'environnement de developpement
3. **Documenter** - Fournir une documentation claire pour les developpeurs
4. **Nettoyer l'historique** (optionnel) - Supprimer definitivement les fichiers sensibles de l'historique git

## Comportement attendu

### Phase 1 - Actions immediates (CRITIQUE)

#### 46.1.1 - Revocation du Client Secret Twitch

**Procedure :**
1. Se connecter sur https://dev.twitch.tv/console/apps
2. Localiser l'application BluePlayer
3. Cliquer sur "New Secret" pour regenerer le `TWITCH_CLIENT_SECRET`
4. **Important** : L'ancien secret est immediatement invalide

**Resultat attendu :**
- L'ancien secret ne fonctionne plus (les attaquants potentiels ne peuvent plus l'utiliser)
- Le nouveau secret est stocke de maniere securisee (localement, jamais commite)

#### 46.1.2 - Regeneration des certificats TLS

**Prerequis :** `mkcert` installe sur la machine

**Procedure :**
```bash
# Installer mkcert si pas deja fait
brew install mkcert
mkcert -install

# Creer le dossier certs/ (ignore par git)
mkdir -p certs

# Generer les nouveaux certificats
cd certs
mkcert -cert-file twitch-cert.pem -key-file twitch-key.pem localhost 127.0.0.1
```

**Resultat attendu :**
- Nouveaux certificats generes localement
- Les anciens certificats (exposes) ne sont plus utilises

#### 46.1.3 - Mise a jour du fichier .env local

**Procedure :**
1. Copier `.env.example` vers `.env` (si le template existe)
2. Remplir les nouvelles valeurs :
   - `TWITCH_CLIENT_ID` - Recupere depuis la console Twitch
   - `TWITCH_CLIENT_SECRET` - Le nouveau secret genere en 46.1.1

**Resultat attendu :**
- L'application fonctionne avec les nouvelles credentials
- Le fichier `.env` n'est pas commite (verifie par `.gitignore`)

### Phase 2 - Documentation et automatisation

#### 46.2.1 - Documentation developpeur

**Creer `docs/DEVELOPMENT_SETUP.md` avec :**

1. **Prerequis**
   - macOS (version minimale)
   - Qt et CMake
   - mkcert
   - Compte developpeur Twitch

2. **Configuration Twitch**
   - Comment creer une application sur dev.twitch.tv
   - Les scopes OAuth necessaires
   - L'URL de redirection OAuth (`https://localhost:3000/callback`)

3. **Generation des certificats TLS**
   - Pourquoi des certificats sont necessaires (OAuth redirect HTTPS)
   - Commandes mkcert detaillees
   - Verification que les certificats fonctionnent

4. **Variables d'environnement**
   - Liste des variables requises
   - Format attendu
   - Ne jamais commiter `.env`

5. **Verification du setup**
   - Comment tester que tout fonctionne
   - Troubleshooting des erreurs courantes

#### 46.2.2 - Script de setup automatise

**Creer `scripts/setup_dev_env.sh` qui :**

1. **Verifie les prerequis**
   - `mkcert` installe ?
   - Qt disponible ?
   - Variables d'environnement de base ?

2. **Genere les certificats TLS**
   - Cree le dossier `certs/` si absent
   - Execute `mkcert` avec les bons parametres
   - Verifie que les fichiers sont crees

3. **Cree le fichier `.env` template**
   - Si `.env` n'existe pas, copie depuis `.env.example`
   - Affiche un message demandant de remplir les valeurs

4. **Affiche un resume**
   - Ce qui a ete configure
   - Les etapes manuelles restantes (obtenir le Client Secret)

**Comportement du script :**
```bash
./scripts/setup_dev_env.sh

# Output attendu :
# [CHECK] mkcert... OK
# [CHECK] certs directory... Created
# [GENERATE] TLS certificates... OK
# [CHECK] .env file... Created from template
#
# Setup complete!
# 
# Next steps:
# 1. Get your Twitch Client ID and Secret from https://dev.twitch.tv/console/apps
# 2. Fill in the values in .env
# 3. Run: cmake -B build && cmake --build build
```

#### 46.2.3 - Test de la procedure from scratch

**Simulation d'un nouveau developpeur :**
1. Clone le repo
2. Execute `./scripts/setup_dev_env.sh`
3. Configure manuellement les credentials Twitch
4. Build et lance l'application
5. Verifie que l'authentification OAuth fonctionne

### Phase 3 - Nettoyage historique git (OPTIONNEL)

> **Attention** : Cette phase est destructrice et necessite une coordination avec tous les contributeurs.

#### 46.3.1 - Backup du repository

```bash
# Creer une copie complete avant toute modification
git clone --mirror https://github.com/user/BluePlayer.git BluePlayer-backup.git
```

#### 46.3.2 - Nettoyage avec BFG Repo-Cleaner

**Prerequis :** Telecharger [BFG Repo-Cleaner](https://rtyley.github.io/bfg-repo-cleaner/)

**Procedure :**
```bash
# Supprimer les fichiers sensibles de TOUT l'historique
java -jar bfg.jar --delete-files .env BluePlayer.git
java -jar bfg.jar --delete-files twitch-cert.pem BluePlayer.git
java -jar bfg.jar --delete-files twitch-key.pem BluePlayer.git

# Nettoyer et compacter
cd BluePlayer.git
git reflog expire --expire=now --all
git gc --prune=now --aggressive
```

#### 46.3.3 - Force push et notification

**Procedure :**
1. Force push sur le remote : `git push --force`
2. Notifier tous les contributeurs de faire un **fresh clone**
3. Invalider toutes les branches/forks existantes

**Message de notification :**
```
ATTENTION: L'historique git a ete reecrit pour supprimer des fichiers sensibles.

Action requise pour tous les contributeurs :
1. Supprimer votre copie locale
2. Faire un fresh clone : git clone https://github.com/user/BluePlayer.git
3. Reconfigurer votre environnement avec ./scripts/setup_dev_env.sh

Les branches non mergees doivent etre rebasees sur le nouveau main.
```

## Fichiers concernes

### A creer
- `docs/DEVELOPMENT_SETUP.md` - Documentation setup developpeur
- `scripts/setup_dev_env.sh` - Script de setup automatise
- `.env.example` - Template des variables d'environnement (si absent)

### A verifier
- `.gitignore` - S'assurer que `.env` et `certs/` sont bien ignores
- `certs/` - Dossier local pour les certificats (ignore par git)

### Deja ignores (a confirmer)
- `.env`
- `certs/twitch-cert.pem`
- `certs/twitch-key.pem`

## Checklist de validation

### Phase 1 - Actions immediates
- [x] Client Secret Twitch regenere sur dev.twitch.tv
- [x] Ancien secret ne fonctionne plus (test de connexion echoue)
- [x] Nouveaux certificats TLS generes avec mkcert
- [x] Fichier `.env` local mis a jour avec les nouvelles valeurs
- [x] Application demarre et OAuth fonctionne

### Phase 2 - Documentation et automatisation
- [x] `docs/DEVELOPMENT_SETUP.md` cree avec toutes les sections
- [x] `.env.example` cree avec les variables (sans valeurs)
- [x] `scripts/setup_dev_env.sh` cree et executable
- [x] Script verifie les prerequis (mkcert)
- [x] Script genere les certificats automatiquement
- [x] Script cree `.env` depuis template si absent
- [x] Documentation testee par un fresh setup

### Phase 3 - Nettoyage historique (ANNULEE)
> Phase annulee - La revocation des secrets rend le nettoyage historique non necessaire.

### Verification finale
- [x] `.gitignore` contient `.env` et `certs/`
- [x] Aucun secret dans les fichiers commites actuels
- [x] README reference `docs/DEVELOPMENT_SETUP.md` (via docs/)
