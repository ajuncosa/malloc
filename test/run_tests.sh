#!/bin/bash

set -o errexit   # abort on nonzero exitstatus
set -o nounset   # abort on unbound variable
set -o pipefail  # don't hide errors within pipes

bindir=$1

for filename in "$bindir"/*; do
    "$filename"
done
