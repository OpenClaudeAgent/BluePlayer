#!/bin/bash

# Script de génération de rapport de couverture de code avec llvm-cov
# Ce script compile le projet avec les flags de couverture, exécute les tests,
# et génère un rapport HTML de couverture.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
COVERAGE_DIR="$PROJECT_ROOT/coverage"
LOAD_ENV_SCRIPT="$SCRIPT_DIR/load_env.sh"

# Vérifier que llvm-profdata et llvm-cov sont disponibles
if ! command -v llvm-profdata &> /dev/null; then
    echo "Erreur: llvm-profdata n'est pas trouvé. Assurez-vous qu'il est installé."
    echo "Sur macOS, il est disponible via Xcode ou LLVM complet."
    exit 1
fi

if ! command -v llvm-cov &> /dev/null; then
    echo "Erreur: llvm-cov n'est pas trouvé. Assurez-vous qu'il est installé."
    echo "Sur macOS, il est disponible via Xcode ou LLVM complet."
    exit 1
fi

echo "=== Génération du rapport de couverture de code ==="
echo ""

# Nettoyer les anciens fichiers de profilage
echo "Nettoyage des anciens fichiers de profilage..."
find "$BUILD_DIR" -name "*.profraw" -delete 2>/dev/null || true
rm -f "$BUILD_DIR"/default.profdata

# Créer le répertoire de couverture
mkdir -p "$COVERAGE_DIR"

# Configurer et compiler avec la couverture activée
echo "Configuration et compilation avec couverture activée..."
cd "$BUILD_DIR"
"$LOAD_ENV_SCRIPT" cmake .. -G Ninja -DCMAKE_PREFIX_PATH="${QT6_DIR}" -DBLUEPLAYER_ENABLE_COVERAGE=ON
"$LOAD_ENV_SCRIPT" cmake --build .

# Définir la variable d'environnement pour la génération de profils
export LLVM_PROFILE_FILE="$BUILD_DIR/%p.profraw"

# Exécuter les tests
echo ""
echo "Exécution des tests avec génération de profils..."
cd "$BUILD_DIR"
"$LOAD_ENV_SCRIPT" ctest --output-on-failure || {
    echo "Attention: Certains tests ont échoué, mais le rapport de couverture sera généré pour les tests réussis."
}

# Trouver tous les fichiers profraw
PROFRAW_FILES=$(find "$BUILD_DIR" -name "*.profraw" 2>/dev/null || true)

if [ -z "$PROFRAW_FILES" ]; then
    echo "Erreur: Aucun fichier de profil (.profraw) trouvé."
    echo "Assurez-vous que les tests ont été exécutés avec les flags de couverture."
    exit 1
fi

echo ""
echo "Fichiers de profil trouvés:"
echo "$PROFRAW_FILES"

# Merger les profils
echo ""
echo "Fusion des profils..."
llvm-profdata merge -sparse "$BUILD_DIR"/*.profraw -o "$BUILD_DIR"/default.profdata

# Trouver les binaires de test et les bibliothèques
TEST_BINARIES=$(find "$BUILD_DIR/tests" -type f -perm +111 -not -name "*.dylib" -not -name "*.so" 2>/dev/null | grep -E "test_|Test" || true)
LIBRARIES=$(find "$BUILD_DIR/src" -name "libblueplayer_*.a" 2>/dev/null || true)

if [ -z "$TEST_BINARIES" ]; then
    echo "Erreur: Aucun binaire de test trouvé."
    exit 1
fi

# Générer le rapport HTML
echo ""
echo "Génération du rapport HTML de couverture..."

# Construire la commande llvm-cov avec tous les objets
COV_OBJECTS=""

# Ajouter les binaires de test
for binary in $TEST_BINARIES; do
    COV_OBJECTS="$COV_OBJECTS -object $binary"
done

# Ajouter les bibliothèques
for lib in $LIBRARIES; do
    COV_OBJECTS="$COV_OBJECTS -object $lib"
done

# Exclure les fichiers générés par Qt et les fichiers de test
EXCLUDE_PATTERNS=(
    "*/moc_*"
    "*/qrc_*"
    "*/uic_*"
    "*/autogen/*"
    "*/build/*"
    "*/tests/*"
    "*/test_*"
    "*/Test*.cpp"
    "*/Test*.hpp"
)

EXCLUDE_ARGS=""
for pattern in "${EXCLUDE_PATTERNS[@]}"; do
    EXCLUDE_ARGS="$EXCLUDE_ARGS -ignore-filename-regex='$pattern'"
done

# Générer le rapport HTML
llvm-cov show \
    -instr-profile="$BUILD_DIR"/default.profdata \
    $COV_OBJECTS \
    $EXCLUDE_ARGS \
    -format=html \
    -output-dir="$COVERAGE_DIR/html" \
    -show-line-counts-or-regions \
    -show-instantiations=false || {
    echo "Erreur lors de la génération du rapport HTML."
    exit 1
}

# Générer le rapport texte
echo ""
echo "Génération du rapport texte..."
llvm-cov report \
    -instr-profile="$BUILD_DIR"/default.profdata \
    $COV_OBJECTS \
    $EXCLUDE_ARGS \
    > "$COVERAGE_DIR/coverage_report.txt" || {
    echo "Erreur lors de la génération du rapport texte."
    exit 1
}

# Afficher un résumé
echo ""
echo "=== Rapport de couverture généré ==="
echo "Rapport HTML: $COVERAGE_DIR/html/index.html"
echo "Rapport texte: $COVERAGE_DIR/coverage_report.txt"
echo ""
echo "Résumé de la couverture:"
head -20 "$COVERAGE_DIR/coverage_report.txt"
