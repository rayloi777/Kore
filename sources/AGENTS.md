# sources - Core Implementation

**Parent:** `./AGENTS.md` | **62 source files, 12 subdirectories**

## OVERVIEW
Core C implementation mirroring includes/kore3 structure. Contains platform-agnostic logic.

## STRUCTURE
```
sources/
├── gpu/          # GPU core logic
├── audio/        # Audio core
├── mixer/        # Mixer implementation
├── math/         # Math library impl
├── input/        # Input handling
├── io/           # File I/O
├── libs/         # Lib bindings
├── 2d/           # 2D rendering
├── framebuffer/ # Framebuffer impl
├── network/     # Networking
├── root/         # Core entry points
└── util/         # Utilities
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| GPU核心 | `sources/gpu/` | Device, buffer, texture logic |
| 数学实现 | `sources/math/` | matrix.c, vector.c |
| 音频 | `sources/mixer/` | Sound streaming, mixing |

## CONVENTIONS (THIS DIR)
- Mirror includes/kore3 structure
- Static functions for internal use
- Cross-platform: use `#if defined(KORE_<PLATFORM>)`

## ANTI-PATTERNS
- NEVER put platform-specific code here (use backends/)
- NEVER skip error checking
