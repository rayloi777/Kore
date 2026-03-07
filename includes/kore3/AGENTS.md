# includes/kore3 - Public API Headers

**Parent:** `./AGENTS.md` | **81 header files, 15 subdirectories`

## OVERVIEW
Public C API headers for Kore3 game engine. Organized by subsystem (gpu, audio, math, etc.).

## STRUCTURE
```
includes/kore3/
├── gpu/          # GPU API (device, buffer, texture, pipeline, sampler)
├── audio/        # Low-level audio API
├── mixer/        # High-level audio mixer
├── math/         # Matrix, vector, quaternion math
├── input/        # Keyboard, mouse, touch input
├── io/           # File I/O, image loading
├── threads/     # Threading primitives
├── network/     # Networking (TCP, UDP)
├── 2d/          # 2D rendering
├── framebuffer/ # Framebuffer management
├── libs/        # Third-party lib bindings
└── *.h          # Core: system, window, display, image, video, log
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| GPU编程 | `gpu/device.h`, `gpu/buffer.h`, `gpu/texture.h` | Main GPU API |
| 音频 | `audio/audio.h` (low), `mixer/mixer.h` (high) | Two levels |
| 数学 | `math/matrix.h`, `math/vector.h` | SIMD available |
| 输入 | `input/keyboard.h`, `input/mouse.h` | Event-based |
| 图像 | `image.h` | PNG, JPEG加载 |

## KEY HEADERS
- `gpu/device.h` — Device creation, command lists
- `gpu/pipeline.h` — Render pipeline state
- `gpu/descriptorset.h` — Resource binding
- `math/matrix.h` — 4x4矩阵 + SIMD优化版本
- `system.h` — 核心系统API

## CONVENTIONS (THIS DIR)
- Header guards: `KORE_<MODULE>_HEADER`
- Use designated initializers for params structs
- `KORE_FUNC` macro for exported functions

## ANTI-PATTERNS
- NEVER include backend-specific headers in public API
- NEVER use `as any` type suppression
