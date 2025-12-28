# Plan 13 - Internationalisation (i18n)

## Contexte

L'application BluePlayer utilise actuellement des textes en français directement dans le code QML. Certains utilisent déjà `qsTr()`, mais la source est en français. Pour une application professionnelle et distribuable internationalement, il faut :
1. Normaliser tous les labels avec `qsTr()` 
2. Utiliser l'anglais comme langue source (convention standard)
3. Créer des fichiers de traduction pour chaque langue
4. Permettre à l'utilisateur de choisir sa langue

## Objectif

Rendre BluePlayer entièrement traduisible avec support initial de :
- **Anglais (en)** : Langue par défaut / source
- **Français (fr)** : Première traduction

## Spécifications

### Architecture i18n Qt

```
src/
  i18n/
    blueplayer_en.ts    # Source (anglais)
    blueplayer_fr.ts    # Traduction française
    blueplayer_en.qm    # Compilé (généré)
    blueplayer_fr.qm    # Compilé (généré)
```

### Logique de sélection de langue

```
1. Vérifier si une préférence utilisateur est définie
   → Si oui, utiliser cette langue
   
2. Sinon, détecter la langue du système (QLocale::system())
   → Si français (fr_FR, fr_CA, fr_BE...) → français
   → Sinon → anglais (fallback)
```

### Labels à migrer

Environ **184 labels uniques** identifiés dans :
- `LoginView.qml` : Textes de connexion
- `HomeView.qml` : Sections, placeholders
- `PlayerView.qml` : Contrôles, statuts
- `PreferencesView.qml` : Paramètres, boutons
- `CacheManagerView.qml` : Gestion des replays
- `VideoPlayerApple.qml` : Lecteur local
- Composants (`components/*.qml`) : Cards, boutons

### Format des clés de traduction

```qml
// AVANT (actuel) - Mauvaise pratique
text: qsTr("Connexion requise")

// APRÈS (cible) - Bonne pratique
text: qsTr("Login required")  // Source en anglais
```

### Fichier de traduction (.ts)

```xml
<!-- blueplayer_fr.ts -->
<message>
    <source>Login required</source>
    <translation>Connexion requise</translation>
</message>
```

## Sous-tâches

| # | Sous-tâche | Description |
|---|------------|-------------|
| 13.1 | Configuration CMake | Intégrer lupdate/lrelease, créer le dossier i18n |
| 13.2 | Normalisation labels | Convertir tous les textes en anglais source avec qsTr() |
| 13.3 | Traduction française | Créer blueplayer_fr.ts avec toutes les traductions |
| 13.4 | Chargement dynamique | Implémenter le chargement des fichiers .qm au démarrage |
| 13.5 | Détection système | Détecter la langue de l'OS et appliquer automatiquement |
| 13.6 | Vérification UI | Tester que les traductions ne cassent pas le layout |
| 13.7 | Sélecteur préférences | Ajouter le choix de langue dans les préférences |

## Priorité des sous-tâches

| Priorité | Sous-tâche | Dépendances |
|----------|------------|-------------|
| 1 | 13.1 - Configuration CMake | Aucune |
| 2 | 13.2 - Normalisation labels | 13.1 |
| 3 | 13.3 - Traduction française | 13.2 |
| 4 | 13.4 - Chargement dynamique | 13.1 |
| 5 | 13.5 - Détection système | 13.4 |
| 6 | 13.6 - Vérification UI | 13.3, 13.5 |
| 7 | 13.7 - Sélecteur préférences | 13.5, **Plan-12** |

## Dépendances externes

| Ce plan | Dépend de | Raison |
|---------|-----------|--------|
| 13.7 | Plan-12 (Préférences) | Le sélecteur de langue sera dans le panel préférences |

**Note** : Les sous-tâches 13.1 à 13.6 peuvent être implémentées indépendamment. Seule la 13.7 nécessite que le plan-12 soit terminé.

## Fichiers concernés

### À créer
- `src/i18n/blueplayer_en.ts`
- `src/i18n/blueplayer_fr.ts`
- `src/i18n/CMakeLists.txt` (optionnel, peut être intégré au CMakeLists principal)

### À modifier
- `CMakeLists.txt` (configuration i18n)
- `src/main.mm` (chargement du QTranslator)
- `src/core/Config.cpp` (stockage préférence langue)
- Tous les fichiers QML avec des labels textuels

### Fichiers QML à migrer (labels)
- `src/ui/LoginView.qml`
- `src/ui/HomeView.qml`
- `src/ui/PlayerView.qml`
- `src/ui/PreferencesView.qml`
- `src/ui/CacheManagerView.qml`
- `src/ui/VideoPlayerApple.qml`
- `src/ui/main.qml`
- `src/ui/components/StreamCard.qml`
- `src/ui/components/ChannelCard.qml`
- `src/ui/components/VideoCard.qml`
- `src/ui/components/ClipCard.qml`
- `src/ui/components/CategoryCard.qml`
- `src/ui/components/LocalPlaybackCard.qml`
- `src/ui/components/PlayerControlBar.qml`
- `src/ui/components/SearchResults.qml`
- `src/ui/components/HorizontalRowSection.qml`

## Détails techniques

### CMake - Configuration i18n

```cmake
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

set(TS_FILES
    src/i18n/blueplayer_en.ts
    src/i18n/blueplayer_fr.ts
)

qt_add_translations(BluePlayer
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
)
```

### main.mm - Chargement traduction

```cpp
#include <QTranslator>
#include <QLocale>

// Dans main()
QTranslator translator;
QString locale = Config::instance().language(); // Préférence utilisateur

if (locale.isEmpty()) {
    // Détection automatique
    locale = QLocale::system().name(); // ex: "fr_FR"
}

if (translator.load("blueplayer_" + locale, ":/i18n")) {
    app.installTranslator(&translator);
}
```

### Points d'attention UI

| Élément | Risque | Solution |
|---------|--------|----------|
| Boutons | Texte trop long | Utiliser `elide: Text.ElideRight` ou largeur min |
| Labels | Overflow | `wrapMode: Text.WordWrap` |
| Menus | Largeur fixe | Largeur dynamique ou `implicitWidth` |
| Placeholders | Coupure | Tester avec textes longs |

## Checklist de validation

### Configuration (13.1)
- [x] CMakeLists.txt configure Qt LinguistTools
- [x] Le dossier `src/i18n/` existe
- [x] `lupdate` génère les fichiers .ts
- [x] `lrelease` compile les fichiers .qm
- [x] Les fichiers .qm sont inclus dans les ressources

### Normalisation (13.2)
- [x] Tous les textes visibles utilisent `qsTr()`
- [x] Les textes sources sont en anglais
- [x] Pas de texte hardcodé en français restant
- [x] Les placeholders dynamiques utilisent `%1`, `%2`, etc.

### Traduction française (13.3)
- [x] Fichier `blueplayer_fr.ts` complet
- [x] Toutes les chaînes sont traduites (pas de "unfinished")
- [x] Les traductions sont naturelles et correctes
- [x] Cohérence du vocabulaire (même terme pour même concept)

### Chargement (13.4)
- [x] QTranslator chargé au démarrage
- [x] La langue s'applique à toute l'UI
- [x] Pas d'erreur si fichier .qm manquant (fallback silencieux)

### Détection système (13.5)
- [x] Langue française détectée sur système fr_FR
- [x] Fallback anglais sur systèmes non-français
- [x] La préférence utilisateur prime sur la détection

### Vérification UI (13.6)
- [x] Aucun texte coupé ou tronqué
- [x] Les boutons restent lisibles
- [x] Les layouts ne cassent pas
- [x] Testé en français ET en anglais

### Sélecteur préférences (13.7)
- [x] Dropdown/liste de langues disponibles
- [x] Changement de langue appliqué immédiatement (ou après redémarrage)
- [x] Préférence sauvegardée
- [x] Préférence chargée au démarrage

### Tests globaux
- [x] L'app démarre en anglais sur système anglais
- [x] L'app démarre en français sur système français
- [x] Le changement manuel de langue fonctionne
- [x] Pas de régression fonctionnelle

## Bonus (ajouté lors de l'implémentation)

- **Changement de langue à chaud** : La langue change immédiatement sans redémarrage grâce au LanguageManager
- **Rafraîchissement des sections Home** : Les titres des sections sont automatiquement mis à jour lors du changement de langue
