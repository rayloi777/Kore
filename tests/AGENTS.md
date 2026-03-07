# tests - Test Applications

**Parent:** `./AGENTS.md` | **18 tests, 15 subdirectories**

## OVERVIEW
Self-contained test/demo applications. Each test is a mini-app demonstrating specific features.

## STRUCTURE
```
tests/
├── cube_test/      # 3D cube + Kongruent shaders ✅
├── triangle/      # 2D triangle ✅
├── texture_test/  # Texture upload + sampling ✅
├── audio_test/    # Audio playback + synthesis ✅
├── matrix_test/   # SIMD matrix benchmarks ✅
├── simd/          # SIMD operations ✅
├── empty/         # Minimal window ✅
├── shader-gpu/    # Shader test ✅
├── shader/        # graphics4 (NOT IMPLEMENTED)
├── shader-g5/     # graphics5 (NOT IMPLEMENTED)
├── input/         # Input handling
├── display/       # Display enumeration
├── multiwindow/   # Multiple windows
├── text_test/     # Text rendering
└── image_compress/# Image compression
```

## WORKING TESTS
| Test | Command | Description |
|------|---------|-------------|
| cube_test | `./make -g metal ... --from tests/cube_test` | 3D rotation, MVP, depth |
| triangle | `./make -g metal ... --from tests/triangle` | 2D colored triangle |
| texture_test | `./make -g metal ... --from tests/texture_test` | Texture upload, RGBA8, Kongruent shader |
| audio_test | `./make -g metal ... --from tests/audio_test` | OGG + sine wave |
| matrix_test | `./make -g metal ... --from tests/matrix_test` | SIMD benchmarks |

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
