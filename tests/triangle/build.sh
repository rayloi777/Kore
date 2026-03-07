#!/bin/bash
cd /Users/rayloi/Downloads/Kore3/Kore3
rm -rf build/Kong-osx-metal-triangle && ./make -g metal --kore . --from tests/triangle --compile
open build/build/Release/Triangle.app
