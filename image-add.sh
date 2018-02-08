#!/bin/bash

set -e
set -x

for i in "$@" ; do
    echidnafs/echfs-utils echidna.img import $i $i
done
