# Plan 22 - Page de succès OAuth

## Contexte

Lors de l'authentification Twitch via OAuth, BluePlayer démarre un serveur HTTP local pour recevoir le callback. Actuellement, une fois le token reçu, la connexion est fermée immédiatement, ce qui provoque une erreur dans le navigateur de l'utilisateur :

```
Ce site est inaccessible
La connexion a été réinitialisée.
ERR_CONNECTION_RESET
```

C'est une mauvaise expérience utilisateur car l'utilisateur ne sait pas si l'authentification a réussi ou échoué.

## Objectif

Afficher une page HTML de confirmation dans le navigateur avant de fermer le serveur, indiquant que l'authentification a réussi et que l'utilisateur peut retourner sur BluePlayer.

## Spécifications

### Page de succès

**URL :** `https://localhost:{port}/callback?code=xxx&...`

**Réponse HTTP actuelle :**
```
(connexion fermée immédiatement → erreur navigateur)
```

**Réponse HTTP souhaitée :**
```http
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8

<!DOCTYPE html>
<html>
<head>
    <title>BluePlayer - Connexion réussie</title>
    <style>
        /* Style minimaliste, thème sombre */
    </style>
</head>
<body>
    <div class="container">
        <h1>✓ Connexion réussie</h1>
        <p>Vous êtes maintenant connecté à Twitch.</p>
        <p>Vous pouvez fermer cet onglet et retourner sur BluePlayer.</p>
    </div>
</body>
</html>
```

### Design de la page

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│                                                         │
│                    ✓ Connexion réussie                  │
│                                                         │
│              Vous êtes maintenant connecté              │
│                      à Twitch.                          │
│                                                         │
│         Vous pouvez fermer cet onglet et                │
│            retourner sur BluePlayer.                    │
│                                                         │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

**Style suggéré :**
- Fond sombre (#1C1C1E) cohérent avec BluePlayer
- Texte blanc/gris clair
- Icône de succès (checkmark vert)
- Police système (San Francisco sur macOS)
- Centré verticalement et horizontalement
- Responsive (fonctionne sur mobile si ouvert là)

### Page d'erreur (optionnel)

Si l'authentification échoue (paramètre `error` présent) :

```html
<h1>✗ Échec de la connexion</h1>
<p>L'authentification a échoué : {error_description}</p>
<p>Veuillez réessayer depuis BluePlayer.</p>
```

### Comportement du serveur

```cpp
// Pseudo-code
void handleCallback(Request req, Response res) {
    if (req.hasParam("code")) {
        // Extraire le code
        QString code = req.param("code");
        
        // Envoyer la page de succès AVANT de traiter le token
        res.setStatus(200);
        res.setHeader("Content-Type", "text/html; charset=utf-8");
        res.send(successPageHtml);
        
        // Puis échanger le code contre un token (async)
        exchangeCodeForToken(code);
        
        // Fermer le serveur après un court délai
        QTimer::singleShot(1000, this, &AuthServer::stop);
        
    } else if (req.hasParam("error")) {
        res.setStatus(400);
        res.send(errorPageHtml);
    }
}
```

### HTML à intégrer

Le HTML peut être :
1. **Hardcodé** dans le C++ (simple, pas de dépendance fichier)
2. **Fichier ressource** Qt (`qrc:/auth/success.html`)
3. **Template** avec placeholders (`{{message}}`)

**Recommandation :** Option 1 (hardcodé) pour la simplicité, c'est une page statique simple.

## Fichiers concernés

### À modifier
- `src/api/twitch/TwitchAuthManager.cpp` - Logique du callback HTTP
- `src/api/twitch/TwitchAuthManager.hpp` - Si besoin de nouvelles méthodes

### Optionnel
- `resources/auth/success.html` - Si on externalise le HTML

## Checklist de validation

### Page de succès
- [ ] La page HTML est retournée avec status 200
- [ ] Le navigateur affiche la page correctement
- [ ] Le message de succès est clair
- [ ] Le style est cohérent avec BluePlayer (thème sombre)
- [ ] L'icône de succès est visible (checkmark ou équivalent)
- [ ] La page est responsive (fonctionne sur différentes tailles)

### Page d'erreur (optionnel)
- [ ] Si `error` présent, afficher page d'erreur
- [ ] Message d'erreur descriptif
- [ ] Invitation à réessayer

### Comportement serveur
- [ ] Le token est toujours correctement extrait
- [ ] L'échange code→token fonctionne toujours
- [ ] Le serveur se ferme après avoir envoyé la réponse
- [ ] Pas de race condition entre réponse et fermeture

### Tests
- [ ] L'authentification complète fonctionne
- [ ] Le navigateur n'affiche plus d'erreur
- [ ] BluePlayer reçoit bien le token
- [ ] Pas de régression sur le flow d'auth existant

## Notes

C'est une amélioration UX simple mais importante pour la première impression de l'application. Un utilisateur qui voit une erreur au premier lancement pourrait penser que quelque chose ne fonctionne pas.
