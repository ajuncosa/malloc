#!/bin/bash

set -o errexit   # abort on nonzero exitstatus
set -o nounset   # abort on unbound variable
set -o pipefail  # don't hide errors within pipes

if [ "$#" -ne 7 ]; then
    echo "Illegal number of parameters"
    exit 1
fi

CURRENT_DIR=$(dirname $0)

python3 "$CURRENT_DIR/rand-generator.py" $2 $3 $4 $5 $6 $7
"$1/rand_test" $2
exit $?