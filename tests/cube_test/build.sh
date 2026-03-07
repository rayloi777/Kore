#!/bin/bash
cd /Users/rayloi/Downloads/Kore3/Kore3
rm -rf build/Kong-osx-metal-cube && ./make -g metal --kore . --from tests/cube_test --compile
open build/build/Release/Cube-Test.app
