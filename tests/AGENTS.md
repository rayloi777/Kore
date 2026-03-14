# tests - Test Applications

**Parent:** `./AGENTS.md` | **19 tests, 16 subdirectories**

## OVERVIEW
Self-contained test/demo applications. Each test is a mini-app demonstrating specific features.

## STRUCTURE
```
tests/
├── cube_test/           # 3D cube + Kongruent shaders ✅
├── cube_texture_test/   # 3D cube + texture mapping ✅
├── triangle/            # 2D triangle ✅
├── texture_test/        # Texture upload + sampling ✅
├── mipmap_test/         # Mipmap levels, sample_lod ✅
├── computeshader_test/  # Compute shader + texture output ✅
├── audio_test/          # Audio playback + synthesis ✅
├── matrix_test/         # SIMD matrix benchmarks ✅
├── simd/                # SIMD operations ✅
├── empty/               # Minimal window ✅
├── shader-gpu/          # Shader test ✅
├── shader/              # graphics4 (NOT IMPLEMENTED)
├── shader-g5/           # graphics5 (NOT IMPLEMENTED)
├── input/               # Input handling
├── display/             # Display enumeration
├── multiwindow/         # Multiple windows
├── text_test/           # Text rendering
├── draw_test/           # Font rendering with stb_truetype ✅
└── image_compress/      # Image compression
```

## WORKING TESTS
| Test | Command | Description |
|------|---------|-------------|
| cube_test | `./make -g metal ... --from tests/cube_test` | 3D rotation, MVP, depth |
| cube_texture_test | `./make -g metal ... --from tests/cube_texture_test` | 3D cube with texture ✅ iOS fixed |
| triangle | `./make -g metal ... --from tests/triangle` | 2D colored triangle |
| texture_test | `./make -g metal ... --from tests/texture_test` | Texture upload, RGBA8, Kongruent shader |
| mipmap_test | `./make -g metal ... --from tests/mipmap_test` | Mipmap levels, sample_lod |
| computeshader_test | `./make -g metal ... --from tests/computeshader_test` | Compute shader, texture output ✅ iOS works |
| draw_test | `./make -g metal ... --from tests/draw_test` | Font rendering, multiple font sizes |
| audio_test | `./make -g metal ... --from tests/audio_test` | OGG + sine wave |
| matrix_test | `./make -g metal ... --from tests/matrix_test` | SIMD benchmarks |

## iOS COMPATIBILITY
Tests with depth textures need resize callback pattern (see `cube_texture_test` for example):
- ✅ `cube_texture_test` - Fixed with resize callback
- ✅ `computeshader_test` - No depth texture, works out of box
- ⚠️ `cube_test`, `triangle`, `text_test`, `image_compress` - Need same fix

## TEST PATTERN
Each test has:
- `sources/main.c` — Entry point with `kickstart()`
- `kfile.js` — Build config
- `shaders/` — Kongruent shaders (.kong)
- `deployment/` — Assets

## BUILD
```bash
./make -g metal --kore . --from tests/[name] --compile
```

## CONVENTIONS (THIS DIR)
- Use Kore3 APIs directly (gpu, audio, system)
- Kongruent shaders for GPU tests
- `kore_init()` + `kore_start()` pattern

## ANTI-PATTERNS
- No formal unit tests — all integration/demo
- DON'T expect test framework — manual verification
