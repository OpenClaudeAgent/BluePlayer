# Patterns et Bonnes Pratiques

Ce document décrit les patterns recommandés et les anti-patterns à éviter pour les tests fonctionnels UI de BluePlayer.

## Table des matières

1. [Structure d'un test](#structure-dun-test)
2. [Patterns recommandés](#patterns-recommandés)
3. [Anti-patterns à éviter](#anti-patterns-à-éviter)
4. [Gestion des signaux](#gestion-des-signaux)
5. [Gestion de l'asynchrone](#gestion-de-lasynchrone)
6. [Tests data-driven](#tests-data-driven)
7. [Helpers et utilitaires](#helpers-et-utilitaires)

---

## Structure d'un test

### Pattern AAA (Arrange-Act-Assert)

Chaque test doit suivre la structure AAA pour une lisibilité maximale :

```qml
function test_volumeButton_click_togglesMute() {
    // Arrange - Préparer le contexte
    var button = findChild(controlBar, "volumeButton")
    verify(button !== null, "volumeButton should exist")
    controlBar.muted = false
    
    // Act - Exécuter l'action
    mouseClick(button)
    
    // Assert - Vérifier le résultat
    compare(muteSpy.count, 1, "muteClicked should be emitted once")
}
```

### Nommage des tests

```qml
// Pattern: test_<élément>_<action>_<résultatAttendu>

// ✅ Bon
function test_playButton_click_emitsPlayPauseSignal() { }
function test_volumeSlider_drag_updatesVolume() { }
function test_speedChip_click_resetsToNormalSpeed() { }

// ❌ Mauvais
function test_button() { }           // Trop vague
function testPlayButton() { }        // Pas de format snake_case
function test_click() { }            // Manque le contexte
```

---

## Patterns recommandés

### 1. Composant inline pour POC / Composant importé pour production

**POC (Proof of Concept)** - Composant simplifié inline :

```qml
Component {
    id: controlBarComponent
    
    Rectangle {
        id: controlBar
        property bool playing: false
        signal playPauseClicked()
        
        // Version simplifiée pour tester la logique
        MouseArea {
            anchors.fill: parent
            onClicked: controlBar.playPauseClicked()
        }
    }
}
```

**Production** - Import du vrai composant :

```qml
import "../../src/ui/components"

Component {
    id: controlBarComponent
    PlayerControlBar {
        // Propriétés de test si nécessaire
    }
}
```

### 2. SignalSpy pour capturer les signaux

```qml
// Déclaration dans le TestCase
SignalSpy { 
    id: playPauseSpy 
    signalName: "playPauseClicked" 
}

function init() {
    // Connecter au composant
    playPauseSpy.target = controlBar
    playPauseSpy.clear()
}

function test_signal_emitted() {
    mouseClick(button)
    
    // Vérifier le nombre d'émissions
    compare(playPauseSpy.count, 1)
    
    // Vérifier les arguments du signal
    compare(playPauseSpy.signalArguments[0][0], expectedValue)
}
```

### 3. createTemporaryObject pour l'isolation

```qml
function init() {
    // Crée une nouvelle instance pour chaque test
    // Automatiquement détruite après le test
    controlBar = createTemporaryObject(controlBarComponent, testCase)
    verify(controlBar !== null)
}

// Pas besoin de cleanup explicite !
```

### 4. waitForRendering pour la synchronisation

```qml
function test_stateChange_updatesVisual() {
    // Changer l'état
    controlBar.playing = true
    
    // Attendre que QML mette à jour le rendu
    waitForRendering(controlBar)
    
    // Maintenant vérifier l'état visuel
    verify(button.color === activeColor)
}
```

### 5. findChild pour localiser les éléments

```qml
function test_nestedElement() {
    // Trouver un élément par objectName
    var button = findChild(controlBar, "playPauseButton")
    verify(button !== null, "Button should exist")
    
    // Interagir avec
    mouseClick(button)
}
```

**Important** : Les éléments doivent avoir un `objectName` :

```qml
// Dans le composant
Rectangle {
    id: playPauseButton
    objectName: "playPauseButton"  // ← Nécessaire pour findChild
}
```

---

## Anti-patterns à éviter

### ❌ Tests qui dépendent de l'ordre

```qml
// MAUVAIS - Le test 2 dépend de l'état laissé par le test 1
function test_1_setPlaying() {
    controlBar.playing = true
}

function test_2_checkPlaying() {
    verify(controlBar.playing)  // Peut échouer si test_1 n'a pas été exécuté
}
```

```qml
// BON - Chaque test est indépendant
function init() {
    controlBar = createTemporaryObject(...)  // État frais
}

function test_setPlaying() {
    controlBar.playing = true
    verify(controlBar.playing)
}
```

### ❌ Assertions vides ou tautologiques

```qml
// MAUVAIS
function test_nothing() {
    mouseClick(button)
    verify(true)  // Ne vérifie rien !
}

function test_tautology() {
    var x = controlBar.volume
    verify(x === x)  // Toujours vrai
}
```

### ❌ Magic numbers

```qml
// MAUVAIS
function test_seek() {
    controlBar.duration = 3600
    controlBar.position = 1800
    // 1800 et 3600 sont des "magic numbers"
}

// BON
function test_seek() {
    var oneHour = 3600
    var halfwayPoint = oneHour / 2
    
    controlBar.duration = oneHour
    controlBar.position = halfwayPoint
}
```

### ❌ Tests trop longs

```qml
// MAUVAIS - Un test qui fait tout
function test_everything() {
    mouseClick(playButton)
    compare(playSpy.count, 1)
    
    mouseClick(volumeButton)
    compare(muteSpy.count, 1)
    
    mouseDrag(seekSlider, ...)
    compare(seekSpy.count, 1)
    
    // ... 50 lignes de plus
}

// BON - Tests focalisés
function test_playButton_click() { ... }
function test_volumeButton_click() { ... }
function test_seekSlider_drag() { ... }
```

### ❌ Délais fixes (sleep)

```qml
// MAUVAIS - Fragile et lent
function test_async() {
    triggerAsyncAction()
    wait(2000)  // Attend 2 secondes peu importe
    verify(result)
}

// BON - Attend une condition
function test_async() {
    triggerAsyncAction()
    tryCompare(target, "property", expectedValue, 2000)  // Timeout de 2s max
}
```

---

## Gestion des signaux

### Vérifier qu'un signal est émis

```qml
SignalSpy { id: spy; signalName: "clicked" }

function test_signalEmitted() {
    spy.target = button
    spy.clear()
    
    mouseClick(button)
    
    compare(spy.count, 1, "Signal should be emitted once")
}
```

### Vérifier les arguments d'un signal

```qml
SignalSpy { id: volumeSpy; signalName: "volumeRequested" }

function test_volumeSignalArguments() {
    // Signal: volumeRequested(real newVolume)
    mouseDrag(slider, 0, 0, 50, 0)
    
    verify(volumeSpy.count >= 1)
    
    // Accéder au premier argument du premier appel
    var requestedVolume = volumeSpy.signalArguments[0][0]
    verify(requestedVolume > 0 && requestedVolume <= 1.0)
}
```

### Vérifier qu'un signal N'est PAS émis

```qml
function test_disabledButton_noSignal() {
    button.enabled = false
    spy.clear()
    
    mouseClick(button)
    
    compare(spy.count, 0, "Signal should NOT be emitted when disabled")
}
```

---

## Gestion de l'asynchrone

### tryCompare pour les valeurs asynchrones

```qml
function test_asyncValue() {
    triggerAsyncOperation()
    
    // Attend que la propriété atteigne la valeur (timeout 5s)
    tryCompare(target, "loading", false, 5000)
}
```

### tryVerify pour les conditions asynchrones

```qml
function test_asyncCondition() {
    triggerAsyncOperation()
    
    // Attend que la condition soit vraie
    tryVerify(function() {
        return target.items.length > 0
    }, 5000)
}
```

### wait() en dernier recours

```qml
function test_animation() {
    triggerAnimation()
    
    // Seulement si tryCompare/tryVerify ne fonctionnent pas
    wait(300)  // Durée de l'animation
    
    verify(target.animationComplete)
}
```

---

## Tests data-driven

Pour tester plusieurs variantes avec le même code :

```qml
function test_speedChange_data() {
    return [
        { tag: "increase", initial: 1.0, delta: 0.25, expected: 1.25 },
        { tag: "decrease", initial: 1.0, delta: -0.25, expected: 0.75 },
        { tag: "max_limit", initial: 2.75, delta: 0.5, expected: 3.0 },
        { tag: "min_limit", initial: 0.5, delta: -0.5, expected: 0.25 },
    ]
}

function test_speedChange(data) {
    // Arrange
    controlBar.playbackRate = data.initial
    
    // Act
    if (data.delta > 0) {
        mouseClick(speedUpButton)
    } else {
        mouseClick(speedDownButton)
    }
    
    // Assert
    compare(rateSpy.signalArguments[0][0], data.expected,
            "Speed change from " + data.initial + " with delta " + data.delta)
}
```

Exécution :
```bash
./test_functional_ui -v2
# Affichera:
# PASS   : test_speedChange(increase)
# PASS   : test_speedChange(decrease)
# PASS   : test_speedChange(max_limit)
# PASS   : test_speedChange(min_limit)
```

---

## Helpers et utilitaires

### Créer un helper réutilisable

```qml
// helpers/TestUtils.qml
pragma Singleton
import QtQuick 2.15

QtObject {
    // Simule un clic avec délai de rendu
    function clickAndWait(item, testCase) {
        testCase.mouseClick(item)
        testCase.waitForRendering(item)
    }
    
    // Vérifie qu'un élément est visible et cliquable
    function verifyClickable(item, testCase) {
        testCase.verify(item !== null, "Item should exist")
        testCase.verify(item.visible, "Item should be visible")
        testCase.verify(item.enabled, "Item should be enabled")
    }
    
    // Formate un temps en mm:ss
    function formatTime(seconds) {
        var m = Math.floor(seconds / 60)
        var s = Math.floor(seconds % 60)
        return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
    }
}
```

### Utiliser le helper

```qml
import "helpers"

TestCase {
    function test_withHelper() {
        TestUtils.verifyClickable(button, this)
        TestUtils.clickAndWait(button, this)
        compare(spy.count, 1)
    }
}
```

---

## Checklist de revue de test

Avant de merger un test, vérifier :

- [ ] Le test suit le pattern AAA
- [ ] Le nommage est descriptif (`test_element_action_result`)
- [ ] Le test est indépendant (pas de dépendance d'ordre)
- [ ] Les assertions vérifient quelque chose de significatif
- [ ] Pas de magic numbers
- [ ] Pas de `wait()` sans justification
- [ ] `waitForRendering()` utilisé après les changements visuels
- [ ] `tryCompare()` utilisé pour les valeurs asynchrones
- [ ] Le test est déterministe (pas de flakiness)
- [ ] Le test s'exécute en < 1 seconde
