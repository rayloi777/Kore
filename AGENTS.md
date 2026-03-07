# AGENTS.md - Kore3 Game Engine

**Generated:** 2026-03-07 | **Branch:** main

## OVERVIEW
Cross-platform C game engine with multi-GPU support (Metal, Vulkan, OpenGL, Direct3D). Uses Kongruent shader language.

## STRUCTURE
```
./
├── includes/kore3/   # Public API headers
├── sources/          # Core implementation
├── backends/gpu/     # GPU backends (metal, vulkan, opengl...)
├── backends/system/  # Platform backends (macos, windows, linux...)
├── tests/            # Test/demo applications
├── tools/            # Build tools (kmake)
└── miniclib/         # Minimal C stdlib
```

## WHERE TO LOOK
| Task | Location |
|------|----------|
| GPU API | `includes/kore3/gpu/` |
| Math | `includes/kore3/math/` (SIMD in `sources/math/`) |
| Audio | `includes/kore3/mixer/` or `audio/` |
| Backend impl | `backends/gpu/metal/`, `backends/system/macos/` |
| Tests | `tests/cube_test/`, `tests/audio_test/` |

## BUILD
```bash
./get_dlc                    # Init submodules
./make -g metal --kore . --from tests/cube_test --compile
open build/build/Release/Cube-Test.app
```

## CODE STYLE
- **Language:** C99/C11, Objective-C (Apple)
- **Naming:** snake_case (functions/vars), SCREAMING_SNAKE_CASE (consts)
- **Pointers:** K&R style `type *var`
- **Format:** `clang-format -style=file -i <file>`

## KEY CONVENTIONS
- Header guards: `KORE_<MODULE>_HEADER`
- Exported: `KORE_FUNC` macro
- Structs: designated initializers `{ .field = value }`
- Includes: kore3/ → backend → kong.h → system (alphabetical)

## ANTI-PATTERNS
- NEVER use `as any` or `@ts-ignore` (no type suppression)
- NEVER put platform code in sources/ (use backends/)
- NEVER mix backends - always use `KORE_METAL`, `KORE_VULKAN` defines
- NO empty catch blocks

## NOTES
- GPU API is working; graphics4/5 not implemented
- Use Kongruent shaders (.kong files) for GPU programs
- Matrix multiply: `multiply(a,b)` = `b * a` (reverse order)
- Use `translation()` + `perspective()` instead of `look_at()` for camera (look_at has issues)
