#!/bin/bash

# Script pour exécuter les tests de mutation avec Mull
# Ce script vérifie la présence de Mull et LLVM 19, puis exécute Mull
# sur les bibliothèques blueplayer_core et blueplayer_media.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
MULL_CONFIG="$PROJECT_ROOT/mull.yml"
COMPILE_COMMANDS="$BUILD_DIR/compile_commands.json"

echo "=== Tests de mutation avec Mull ==="
echo ""

# Vérifier que LLVM 19 est installé
if ! command -v llvm-config-19 &> /dev/null && ! brew list llvm@19 &> /dev/null; then
    echo "Erreur: LLVM 19 n'est pas installé."
    echo "Installez-le avec: brew install llvm@19"
    exit 1
fi

# Déterminer le chemin de llvm-config
if command -v llvm-config-19 &> /dev/null; then
    LLVM_CONFIG="llvm-config-19"
elif [ -f "/opt/homebrew/opt/llvm@19/bin/llvm-config" ]; then
    LLVM_CONFIG="/opt/homebrew/opt/llvm@19/bin/llvm-config"
elif [ -f "/usr/local/opt/llvm@19/bin/llvm-config" ]; then
    LLVM_CONFIG="/usr/local/opt/llvm@19/bin/llvm-config"
else
    echo "Erreur: llvm-config pour LLVM 19 n'est pas trouvé."
    exit 1
fi

LLVM_VERSION=$($LLVM_CONFIG --version | cut -d. -f1)
if [ "$LLVM_VERSION" != "19" ]; then
    echo "Attention: Version LLVM détectée: $LLVM_VERSION, mais LLVM 19 est requis pour Mull."
fi

# Vérifier que Mull est installé
MULL_RUNNER=""
if command -v mull-runner-19 &> /dev/null; then
    MULL_RUNNER="mull-runner-19"
elif command -v mull-runner &> /dev/null; then
    MULL_RUNNER="mull-runner"
    echo "Attention: Utilisation de mull-runner au lieu de mull-runner-19"
else
    echo "Erreur: Mull n'est pas installé."
    echo ""
    echo "Pour installer Mull:"
    echo "1. Téléchargez les binaires depuis: https://github.com/mull-project/mull/releases"
    echo "2. Ou compilez depuis les sources: https://github.com/mull-project/mull"
    echo ""
    echo "Assurez-vous que mull-runner-19 (ou mull-runner) est dans votre PATH."
    exit 1
fi

echo "Mull trouvé: $MULL_RUNNER"
echo "LLVM config: $LLVM_CONFIG"
echo ""

# Vérifier que compile_commands.json existe
if [ ! -f "$COMPILE_COMMANDS" ]; then
    echo "Erreur: compile_commands.json n'existe pas dans $BUILD_DIR"
    echo "Assurez-vous que le projet a été configuré avec CMAKE_EXPORT_COMPILE_COMMANDS=ON"
    echo "Reconfiguration du projet..."
    cd "$BUILD_DIR"
    "$SCRIPT_DIR/load_env.sh" cmake .. -G Ninja -DCMAKE_PREFIX_PATH="${QT6_DIR}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

# Vérifier que le fichier de configuration Mull existe
if [ ! -f "$MULL_CONFIG" ]; then
    echo "Erreur: Fichier de configuration Mull non trouvé: $MULL_CONFIG"
    exit 1
fi

# Vérifier que les tests ont été compilés
if [ ! -d "$BUILD_DIR/tests" ]; then
    echo "Erreur: Le répertoire de tests n'existe pas. Compilez d'abord le projet."
    exit 1
fi

# Trouver les binaires de test
TEST_BINARIES=$(find "$BUILD_DIR/tests" -type f -perm +111 -not -name "*.dylib" -not -name "*.so" 2>/dev/null | grep -E "test_|Test" || true)

if [ -z "$TEST_BINARIES" ]; then
    echo "Erreur: Aucun binaire de test trouvé dans $BUILD_DIR/tests"
    exit 1
fi

echo "Binaires de test trouvés:"
echo "$TEST_BINARIES"
echo ""

# Créer le répertoire pour les rapports de mutation
MUTATION_REPORTS_DIR="$PROJECT_ROOT/mutation-reports"
mkdir -p "$MUTATION_REPORTS_DIR"

# Exécuter Mull
echo "Exécution de Mull..."
echo ""

# Construire la commande Mull
MULL_CMD="$MULL_RUNNER"

# Ajouter les options de configuration
MULL_CMD="$MULL_CMD --config $MULL_CONFIG"
MULL_CMD="$MULL_CMD --compilation-database $COMPILE_COMMANDS"
MULL_CMD="$MULL_CMD --reporters=IDE"
MULL_CMD="$MULL_CMD --reporters=HTML:$MUTATION_REPORTS_DIR"
MULL_CMD="$MULL_CMD --reporters=JSON:$MUTATION_REPORTS_DIR/mutation-report.json"

# Ajouter les binaires de test
for binary in $TEST_BINARIES; do
    MULL_CMD="$MULL_CMD $binary"
done

# Exécuter Mull
cd "$PROJECT_ROOT"
eval "$MULL_CMD" || {
    EXIT_CODE=$?
    echo ""
    echo "Mull a terminé avec le code de sortie: $EXIT_CODE"
    echo "Cela peut indiquer des mutations détectées (ce qui est normal pour un mutation testing)."
    echo "Consultez les rapports dans: $MUTATION_REPORTS_DIR"
    exit $EXIT_CODE
}

echo ""
echo "=== Tests de mutation terminés ==="
echo "Rapports disponibles dans: $MUTATION_REPORTS_DIR"
