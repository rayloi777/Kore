# backends/gpu - GPU Backend Implementations

**Parent:** `./AGENTS.md` | **221 files, 7 subdirectories**

## OVERVIEW
Platform-specific GPU API implementations (Metal, Vulkan, OpenGL, Direct3D, WebGPU).

## STRUCTURE
```
backends/gpu/
├── metal/        # Apple Metal (PRIMARY)
├── vulkan/       # Vulkan (cross-platform)
├── opengl/       # OpenGL (legacy)
├── direct3d11/   # D3D11 (Windows)
├── direct3d12/   # D3D12 (Windows)
├── kompjuta/     # Compute-only backend
└── webgpu/      # WebGPU (browser)
```

## WHERE TO LOOK
| Backend | Primary Use | Status |
|---------|-------------|--------|
| metal/ | macOS, iOS | ✅ Primary |
| vulkan/ | Linux, Windows, Android | ✅ Working |
| opengl/ | Legacy | ✅ Working |
| direct3d11/ | Windows | ✅ Working |
| webgpu/ | Browser | ⚠️ Limited |

## KEY FILES
- `metal/device.m` — Metal device implementation
- `metal/commandlist.m` — Command list encoding
- `vulkan/device.c` — Vulkan device/pipeline

## CONVENTIONS (THIS DIR)
- Backend dir must have: `device`, `commandlist`, `pipeline` implementations
- Use conditional: `#if defined(KORE_<BACKEND>)`
- Metal uses Objective-C (.m), others use C

## ANTI-PATTERNS
- NEVER mix backend logic in sources/ (cross-platform)
- NEVER assume single-backend — always use KORE_* defines
