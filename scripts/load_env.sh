#!/bin/bash

# Charger les variables d'environnement depuis .env
if [ -f .env ]; then
    export $(grep -v '^#' .env | xargs)
fi

# Ajoutez le dossier scripts au PATH
export PATH="$BLUEPLAYER_ROOT/scripts:$PATH"

# Exécuter la commande passée en argument
exec "$@"
