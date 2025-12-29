# Plan 42 - Fix TypeError ChatMessage.qml

## Contexte

Dans les logs de l'application, un warning TypeError est repete des centaines de fois par session :

```
[WARN] qrc:/qt/qml/BluePlayer/ui/components/ChatMessage.qml:85: TypeError: Cannot read property 'length' of null
```

Sur une session de 20 minutes, ce warning apparait **893 fois**, representant 35% du volume total des logs. Ce bug genere du bruit inutile et masque les vrais problemes.

## Objectif

Corriger le bug de null-check dans `ChatMessage.qml` ligne 85 pour eliminer ces warnings repetes.

## Comportement attendu

**Avant (bug) :**
```qml
model: root.emoteParts.length > 0 ? root.emoteParts : [{type: "text", content: root.rawMessage}]
```

Si `emoteParts` est `null` (pas un tableau vide), l'acces a `.length` echoue.

**Apres (corrige) :**
```qml
model: (root.emoteParts && root.emoteParts.length > 0) ? root.emoteParts : [{type: "text", content: root.rawMessage}]
```

Verifier que `emoteParts` existe avant d'acceder a `.length`.

## Fichiers concernes

- `src/ui/components/ChatMessage.qml` (ligne 85)

## Checklist de validation

- [x] La ligne 85 inclut une verification de null avant `.length`
- [x] Aucun warning TypeError dans les logs lors de la lecture du chat
- [x] Les messages du chat s'affichent correctement (texte et emotes)
- [x] Test avec un stream ayant beaucoup de messages pour confirmer
