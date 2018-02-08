#!/bin/bash

set -e
set -x

IMAGE="$1"
ECHFS_UTILS="echfs-utils"

if [ "$IMAGE" == "" ]; then
    exit 1
fi

for i in "$@" ; do
    if [ "$i" == "$IMAGE" ]; then
        continue
    fi
    "$ECHFS_UTILS" "$IMAGE" import "$i" "$i"
done
