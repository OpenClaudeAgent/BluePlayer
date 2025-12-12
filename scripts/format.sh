#!/bin/bash

# Script de formatage du code avec clang-format

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Vérifier si clang-format est disponible
if ! command -v clang-format &> /dev/null; then
    echo "Erreur: clang-format n'est pas installé."
    echo "Installez-le avec: brew install clang-format"
    exit 1
fi

echo "Formatage du code avec clang-format..."

# Formater tous les fichiers C++/C dans src/
find "$PROJECT_ROOT/src" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" -o -name "*.mm" \) -exec clang-format -i {} +

echo "Formatage terminé."












