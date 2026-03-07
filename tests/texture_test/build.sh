#!/bin/bash
cd /Users/rayloi/Downloads/Kore3/Kore3
rm -rf build/Kong-osx-metal-texture_test && ./make -g metal --kore . --from tests/texture_test --compile
open build/build/Release/Texture-Test.app
