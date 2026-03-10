# AGENTS.md - Kore3 Game Engine

**Generated:** 2026-03-10 | **Branch:** main

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
| Tests | `tests/cube_test/`, `tests/texture_test/`, `tests/audio_test/`, `tests/mipmap_test/`, `tests/computeshader_test/`, `tests/g2_test/`, `tests/raytracing_cornellbox/` |

## BUILD
```bash
./get_dlc                    # Init submodules
./make -g metal --kore . --from tests/computeshader_test --compile
open build/build/Release/computeshader_test.app
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
- Metal texture sampling requires `KORE_GPU_TEXTURE_USAGE_SAMPLED` flag in texture creation
- Image loading: stbi converts all formats to RGBA when comp=4, RGB images need alpha set to opaque
- g2 2D rendering: orthographic projection matrix for screen coords → NDC
  - kore_matrix3x3 is column-major: m[x*3+y] = column x, row y
  - 2D ortho matrix (screen 0,width × 0,height → NDC -1,1): m[0]=2/width, m[4]=2/height, m[2]=-1, m[5]=-1

## iOS DEVELOPMENT
- **Build:** `./make -t ios -g metal --kore . --from <test> --compile --nosigning`
- **Simulator:** Requires separate xcodebuild for simulator target
- **IMPORTANT:** iOS ignores `width`/`height` passed to `kore_init()` - actual size comes from `kore_window_width(0)`/`kore_window_height(0)` which are set by `layoutSubviews`
- **Depth textures:** MUST use resize callback pattern - do NOT create with hardcoded dimensions in `kickstart()`
  ```c
  static void resize(int w, int h, void *data) {
      // Create/recreate depth texture with actual window size
      kore_gpu_texture_parameters depth_params = {
          .width = w, .height = h, ...
      };
      kore_gpu_device_create_texture(&device, &depth_params, &depth_texture);
  }
  // In kickstart():
  kore_window_set_resize_callback(0, resize, NULL);
  ```
- **Affected tests:** cube_texture_test, cube_test, triangle, text_test, image_compress all need this fix

## RAYTRACING (METAL)
- **Test:** `tests/raytracing_cornellbox/` - Cornell Box scene with rotating cube, mirror, and floor
- **Build:** `./make -g metal --kore . --from tests/raytracing_cornellbox --compile`

### Completed Fixes

**Kore Metal backend (`backends/gpu/metal/sources/pipeline.m`):**
- Updated `kore_metal_ray_pipeline_init` to use `MTLRayTracingPipelineState` with `MTLRayTracingPipelineDescriptor`
- Added compile-time check `#if __has_include(<Metal/MTLRayTracing.h>)` to disable ray tracing when API unavailable

**Kongruent Metal backend (`Kongruent/sources/backends/metal.c`):**
- Added ray pipeline support (`#[raypipe]` - raygen, raymiss, rayclosesthit, rayintersection, rayanyhit)
- Added rayset type detection for shader parameters
- Fixed `var_name()` to handle globals with sets_count == 0
- Fixed function signature generation for ray shaders (no descriptor buffer params)

**Kongruent integration (`Kongruent/sources/integrations/kore3.c`):**
- Fixed `kong_set_ray_pipeline_*` to use correct API: `kore_metal_command_list_set_ray_pipeline` instead of hardcoded D3D12
- Fixed Metal ray pipeline parameter names: use `.gen_shader.function_name` instead of `.gen_shader_name`
- Fixed Metal ray pipeline init call: 3 args (no root signature) instead of 4 args

### Current Limitations

1. **Metal Ray Tracing API not available:** The Metal Ray Tracing API (`MTLRayTracingPipelineDescriptor`, `MTLHitGroupDescriptor`, `MTLRayTracingPipelineState`) is not available in the current Xcode SDK (macOS 26.2). The ray pipeline code is stubbed out with `#if __has_include(<Metal/MTLRayTracing.h>)` until Apple releases the API.

2. **Metal shader errors:** The generated Metal shader has compilation errors:
   - Type mismatches (uint3 vs float3)
   - Missing `min`/`max` members on ray type
   - Undeclared `ray` variable in miss shader
   - Ambiguous `fmod` call (needs explicit float type)
   
   These are Kongruent shader compiler issues that need to be fixed in the shader code generation.
