#!/usr/bin/env bash

. `dirname "$0"`/tools/platform.sh
MAKE="`dirname "$0"`/tools/$KORE_PLATFORM/kmake$KORE_EXE_SUFFIX"

REBUILD_SHADERS=false
NEW_ARGS=""

# Parse arguments for --rebuild-shaders
for arg in "$@"; do
    if [ "$arg" = "--rebuild-shaders" ]; then
        REBUILD_SHADERS=true
    else
        if [ -z "$NEW_ARGS" ]; then
            NEW_ARGS="$arg"
        else
            NEW_ARGS="$NEW_ARGS $arg"
        fi
    fi
done

if [ "$REBUILD_SHADERS" = true ]; then
    echo "Forcing shader rebuild..."
    rm -rf build/Kong-osx-metal*
fi

if [ -f "$MAKE" ]; then
    exec $MAKE $NEW_ARGS
else 
    echo "kmake was not found, please run the get_dlc script."
    exit 1
fi
