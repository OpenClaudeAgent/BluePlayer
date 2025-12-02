#!/bin/bash

# Script de validation complète du build
# Ce script compile le projet, exécute tous les tests, génère le rapport de couverture
# et vérifie qu'un seuil minimum de couverture (70%) est atteint.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
COVERAGE_DIR="$PROJECT_ROOT/coverage"
COVERAGE_THRESHOLD=70

echo "=== Validation complète du build ==="
echo ""

# Étape 1: Compiler le projet
echo "Étape 1/4: Compilation du projet..."
cd "$BUILD_DIR"
"$SCRIPT_DIR/load_env.sh" cmake .. -G Ninja -DCMAKE_PREFIX_PATH="${QT6_DIR}" -DBLUEPLAYER_ENABLE_COVERAGE=ON
"$SCRIPT_DIR/load_env.sh" cmake --build .

if [ $? -ne 0 ]; then
    echo "Erreur: La compilation a échoué."
    exit 1
fi

echo "✓ Compilation réussie"
echo ""

# Étape 2: Exécuter les tests
echo "Étape 2/4: Exécution des tests..."
cd "$BUILD_DIR"
"$SCRIPT_DIR/load_env.sh" ctest --output-on-failure

if [ $? -ne 0 ]; then
    echo "Erreur: Certains tests ont échoué."
    exit 1
fi

echo "✓ Tous les tests ont réussi"
echo ""

# Étape 3: Générer le rapport de couverture
echo "Étape 3/4: Génération du rapport de couverture..."
"$SCRIPT_DIR/generate_coverage.sh"

if [ $? -ne 0 ]; then
    echo "Erreur: La génération du rapport de couverture a échoué."
    exit 1
fi

echo "✓ Rapport de couverture généré"
echo ""

# Étape 4: Vérifier le seuil de couverture
echo "Étape 4/4: Vérification du seuil de couverture (${COVERAGE_THRESHOLD}%)..."

COVERAGE_REPORT="$COVERAGE_DIR/coverage_report.txt"

if [ ! -f "$COVERAGE_REPORT" ]; then
    echo "Erreur: Le rapport de couverture n'existe pas: $COVERAGE_REPORT"
    exit 1
fi

# Extraire le pourcentage de couverture depuis le rapport
# Le rapport llvm-cov report contient une ligne avec "TOTAL" et les pourcentages
COVERAGE_LINE=$(grep -i "TOTAL" "$COVERAGE_REPORT" | tail -1 || true)

if [ -z "$COVERAGE_LINE" ]; then
    echo "Attention: Impossible d'extraire le pourcentage de couverture depuis le rapport."
    echo "Contenu du rapport:"
    cat "$COVERAGE_REPORT"
    exit 1
fi

# Extraire le pourcentage de lignes couvertes (généralement la 3ème colonne)
# Format typique: "TOTAL    1234    567    890    45.90%"
COVERAGE_PERCENT=$(echo "$COVERAGE_LINE" | awk '{print $NF}' | sed 's/%//')

if [ -z "$COVERAGE_PERCENT" ]; then
    echo "Attention: Impossible d'extraire le pourcentage de couverture."
    echo "Ligne trouvée: $COVERAGE_LINE"
    exit 1
fi

echo "Couverture actuelle: ${COVERAGE_PERCENT}%"
echo "Seuil requis: ${COVERAGE_THRESHOLD}%"

# Comparer avec le seuil (utiliser awk pour la comparaison de nombres décimaux)
COVERAGE_CHECK=$(echo "$COVERAGE_PERCENT $COVERAGE_THRESHOLD" | awk '{if ($1 >= $2) print "OK"; else print "FAIL"}')

if [ "$COVERAGE_CHECK" != "OK" ]; then
    echo ""
    echo "❌ ERREUR: Le seuil de couverture n'est pas atteint!"
    echo "Couverture actuelle: ${COVERAGE_PERCENT}%"
    echo "Seuil requis: ${COVERAGE_THRESHOLD}%"
    echo ""
    echo "Consultez le rapport détaillé: $COVERAGE_DIR/html/index.html"
    exit 1
fi

echo "✓ Le seuil de couverture est atteint"
echo ""

echo "=== Validation complète réussie ==="
echo ""
echo "Résumé:"
echo "  - Compilation: ✓"
echo "  - Tests: ✓"
echo "  - Couverture: ${COVERAGE_PERCENT}% (seuil: ${COVERAGE_THRESHOLD}%)"
echo ""
echo "Rapport de couverture: $COVERAGE_DIR/html/index.html"
