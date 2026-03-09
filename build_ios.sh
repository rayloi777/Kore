#!/bin/bash

PROJECT_PATH="${1:-tests/cube_texture_test}"
NOSIGNING="${2:---nosigning}"

if [ "$NOSIGNING" = "--nosigning" ]; then
    ./make -t ios -g metal --kore . --from "$PROJECT_PATH" --compile --nosigning
else
    ./make -t ios -g metal --kore . --from "$PROJECT_PATH" --compile
fi
