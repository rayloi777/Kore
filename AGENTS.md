# AGENTS.md - Kore Game Engine

Guidelines for agentic coding agents operating in the Kore repository.

## Build Commands

### Prerequisites
```bash
./get_dlc  # Initialize git submodules (downloads kmake build tools)
xcodebuild -downloadComponent MetalToolchain  # Download Metal toolchain for shader compilation
```

### Single Test Commands

#### macOS (Metal)
```bash
./make -g metal --kore . --from tests/empty --compile
open build/build/Release/Empty.app
```

#### macOS (Metal) - Cube Test
```bash
./make -g metal --kore . --from tests/cube_test --compile
open build/build/Release/Cube-Test.app
```

#### macOS (OpenGL)
```bash
./make -g opengl --kore . --from tests/empty --compile
```

#### iOS Simulator
```bash
./make -g metal --kore . --from tests/shader-gpu -t ios --compile --nosigning
# Then open build/Shader-GPU.xcodeproj in Xcode and run on simulator
```

#### Android
```bash
./make -g vulkan --kore . --from tests/empty -t android --compile
```

#### Web (WebGPU via Emscripten)
```bash
./make -g webgpu --kore . --from tests/empty --compile --server
# Open http://localhost:8080 in browser
```

### Available Tests
| Test | Description | Status |
|------|-------------|--------|
| `tests/empty` | Minimal window | ✅ |
| `tests/shader-gpu` | Shader test | ✅ |
| `tests/simd` | SIMD operations | ✅ |
| `tests/cube_test` | Rotating cube with colors | ✅ |
| `tests/triangle_test` | Simple 2D triangle | ✅ |
| `tests/texture_test` | Texture upload + sampling | ✅ |
| `tests/shader-g5` | graphics5 API | ❌ Not implemented |
| `tests/shader` | graphics4 API | ❌ Not implemented |

### Build texture_test
```bash
./make -g metal --kore . --from tests/texture_test --compile
open build/build/Release/Texture-Test.app
```

### Build triangle_test
```bash
./make -g metal --kore . --from tests/triangle --compile
open build/build/Release/Triangle.app
```

### Code Formatting
```bash
clang-format -style=file -i <file>           # Format single file
node ./.github/format.js                      # Format entire codebase
```

## Code Style

### Language & Standard
- **Language**: C (Objective-C for Apple platforms)
- **Standard**: C99/C11
- **Formatting**: Use clang-format with `.clang-format`

### Naming Conventions
| Element | Convention | Example |
|---------|------------|---------|
| Functions | snake_case | `kore_gpu_device_create` |
| Variables | snake_case | `vertex_buffer` |
| Constants/Enums | SCREAMING_SNAKE_CASE | `KORE_GPU_LOAD_OP_CLEAR` |
| Structs | snake_case (prefixed) | `kore_gpu_device` |
| Typedefs | snake_case + _t suffix | `kore_gpu_command_list_t` |
| Files | snake_case | `device.h` |

### Pointer Alignment (K&R Style)
```c
// CORRECT
kore_gpu_device *device;

// WRONG
kore_gpu_device* device;
```

### Function Declarations
```c
// Exported functions - use KORE_FUNC macro
KORE_FUNC void kore_gpu_device_create(kore_gpu_device *device, const kore_gpu_device_wishlist *wishlist);

// Static functions
static void update(void *data) { }
```

### Struct Initialization
Always use designated initializers:
```c
kore_gpu_color clear_color = {
    .r = 0.0f,
    .g = 0.0f,
    .b = 0.0f,
    .a = 1.0f,
};
```

### Header File Structure
```c
#ifndef KORE_<MODULE>_HEADER
#define KORE_<MODULE>_HEADER

#include <kore3/global.h>

#ifdef __cplusplus
extern "C" {
#endif

// ... declarations

#ifdef __cplusplus
}
#endif
#endif
```

### Include Order (Alphabetical)
1. Public Kore headers (`<kore3/...>`)
2. Backend-specific headers (`<kore3/metal/...>`)
3. Kong header (`<kong.h>`)
4. System C library headers (`<assert.h>`, `<stdlib.h>`)

```c
#include <kore3/gpu/device.h>
#include <kore3/io/filereader.h>
#include <kong.h>
#include <assert.h>
```

### Conditional Compilation
```c
#if defined(KORE_METAL)
    #include <kore3/metal/device_structs.h>
#elif defined(KORE_OPENGL)
    #include <kore3/opengl/device_structs.h>
#elif defined(KORE_VULKAN)
    #include <kore3/vulkan/device_structs.h>
#else
    #error("Unknown GPU backend")
#endif
```

### Error Handling
- Return error codes or boolean
- Use assertions for development (`assert.h`)
- Check return values from GPU operations

### Comments
- Use `//` for single-line, `/* ... */` for multi-line
- Document public APIs with Doxygen-style comments

## Architecture

### Key Directories
- `includes/kore3/` - Public API headers
- `sources/` - Core implementation
- `backends/gpu/` - GPU backend implementations (metal, opengl, vulkan, webgpu, etc.)
- `backends/system/` - Platform-specific system code

### Adding New API Functions
1. Add declaration to header in `includes/kore3/`
2. Add implementation to source in `sources/`
3. Add backend-specific implementation in `backends/gpu/<backend>/sources/`
4. Format: `clang-format -style=file -i <files>`

## API Status

| API | Status |
|-----|--------|
| **gpu** | ✅ Working - Use `includes/kore3/gpu/*` |
| graphics4 | ❌ Not implemented |
| graphics5 | ❌ Not implemented |

## Recent Fixes

### GPU API Enhancements
- Added `includes/kore3/gpu/pipeline.h` - Unified pipeline types
- Added `kore_gpu_buffer_unlock_all()` - Buffer unlock function
- Added `includes/kore3/gpu/error.h` - Error handling system with error codes
- Added `kore_gpu_sampler` support - basic + mipmap + anisotropy
- Added `KORE_GPU_BUFFER_USAGE_VERTEX` flag
- Added texture usage flags: `KORE_GPU_TEXTURE_USAGE_SAMPLED`, `KORE_GPU_TEXTURE_USAGE_STORAGE`

### Descriptor Set API (NEW)
- Added `includes/kore3/gpu/descriptorset.h` - Public API for resource binding
- Types: `kore_gpu_descriptor_set_layout`, `kore_gpu_descriptor_set`, `kore_gpu_descriptor_binding`
- Functions: create/destroy layout, create/destroy set, set_buffer/set_texture/set_sampler
- Metal backend: ✅ Implemented
- Vulkan backend: ✅ Framework complete

### texture_test
- Created `tests/texture_test/` test project
- Basic quad rendering with UV coordinate mapping
- Uploaded texture data using COPY command list
- Displays loaded `haxe.png` image with texture sampling
- Uses Kongruent shader with `sample(tex, sam, uv)` function

### Kongruent Shader Syntax
- Fixed `sources/backends/metal.c` in Kongruent:
  - `var_name()` function: Changed hardcoded `argument_buffer0` to use actual set index
  - `sample()` call generation: Fixed to use correct set indices for texture and sampler

### cube_test Features
- 24 vertices with different colors per face (red, green, blue, yellow, purple, cyan)
- 36 indices for 12 triangles (2 per face)
- MVP matrix transformation with rotation animation (X and Y axes)
- Depth buffer testing
- Back-face culling
- **Kongruent shader integration** - Uses `shaders/shaders.kong`
- Fixed vertex buffer API to use proper Kong helper functions
 pointer type mismatch in- Fixed Metal backend `kore_metal_command_list_set_vertex_buffer`

### triangle_test (NEW)
- Created `tests/triangle/` test project
- Simple 2D triangle with 3 vertices
- Each vertex has a different color (red, green, blue)
- Uses identity matrix (no transformation)
- Demonstrates basic Kong API usage

### Bug Fixes
- Fixed `kore_fb_init()` function prototype (added `void` parameter)
- Fixed uninitialized `uniforms` variable warning in tests

### triangle_test Features
- 3 vertices for a simple 2D triangle
- Each vertex has a different color (red, green, blue)
- Colors are interpolated across the triangle surface
- Uses identity matrix (no transformation)
- Simple Kongruent shader similar to cube_test
- Demonstrates basic vertex buffer and rendering pipeline

### Kongruent Shader Usage
The cube_test now uses Kongruent shaders (Kore's shader language) instead of hardcoded Metal shaders:

**`tests/cube_test/shaders/shaders.kong`:**
```kong
struct vertex_in {
    pos: float3;
    col: float3;
}

struct vertex_out {
    pos: float4;
    col: float3;
}

#[set(everything)]
const constants: {
    mx4;
};

fun pos(input: vertex_in): vertex_out {vp: float4 ... }
fun pix(input: vertex_out): float4 { ... }

#[pipe]
struct pipeline {
    vertex = pos;
    fragment = pix;
    depth_stencil_format = TEXTURE_FORMAT_DEPTH32_FLOAT;
    depth_write = true;
    depth_compare = COMPARE_LESS;
    format = framebuffer_format();
}
```

**Key Kongruent conventions:**
- Vertex function named `pos`, fragment function named `pix`
- Uniforms via `#[set(everything)]` block
- Use `constants_type` from generated `kong.h` for uniform buffers
- Use Kong helper functions: `kong_init()`, `kong_set_render_pipeline_pipeline()`, `kong_set_vertex_buffer_vertex_in()`, `kong_set_descriptor_set_everything()`, `constants_type_buffer_create()`

## Example: New GPU API Usage
```c
#include <kore3/gpu/device.h>
#include <kong.h>

kore_gpu_device device;
kore_gpu_command_list list;

kore_gpu_device_wishlist wishlist = {0};
kore_gpu_device_create(&device, &wishlist);
kong_init(&device);
kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);
```

## Example: cube_test (Rendering a Cube with Kongruent Shaders)

See `tests/cube_test/sources/main.c` for a complete example including:
- Kongruent shader loading from `shaders/shaders.kong`
- Kong initialization (`kong_init()`)
- Vertex buffer creation with position + color attributes
- Index buffer for triangle indices
- Uniform buffer using `constants_type_buffer_create()`
- Descriptor set via `kong_create_everything_set()`
- Render pipeline via `kong_set_render_pipeline_pipeline()`
- MVP matrix computation with rotation animation
- Depth texture for proper depth testing

**Key Kong API calls:**
```c
kong_init(&device);
constants_type_buffer_create(&device, &uniform_buffer, 1);
kong_create_everything_set(&device, &params, &uniform_set);

// In render loop:
kong_set_render_pipeline_pipeline(&list);
kong_set_vertex_buffer_vertex_in(&list, &vertex_buffer);
kong_set_descriptor_set_everything(&list, &uniform_set);
```

## Example: triangle_test (Simple 2D Triangle)

See `tests/triangle/sources/main.c` for a complete example including:
- 3 vertices with position (float3) and color (float3)
- Simple identity matrix (no transformation)
- Uses same Kongruent shader pattern as cube_test

**Vertex data:**
```c
static vertex_in vertices[3] = {
    { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },  // Top - Red
    { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },  // Bottom left - Green
    { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },  // Bottom right - Blue
};
```

**Key Kong API calls:**
```c
// Create vertex buffer using Kong helper
kong_create_buffer_vertex_in(&device, 3, &vertex_buffer);
{
    vertex_in *ptr = kong_vertex_in_buffer_lock(&vertex_buffer);
    memcpy(ptr, vertices, sizeof(vertices));
    kong_vertex_in_buffer_unlock(&vertex_buffer);
}

// In render loop:
kong_set_render_pipeline_pipeline(&list);
kong_set_vertex_buffer_vertex_in(&list, &vertex_buffer);
kong_set_descriptor_set_everything(&list, &uniform_set);
kore_gpu_command_list_draw_indexed(&list, 3, 1, 0, 0, 0);
```

## Kongruent Shader Syntax

### Multiple Constants in Same Set
Each `const` declaration needs its own `#[set(...)]` attribute, even when in the same set:
```kong
#[set(textures)]
const tex: tex2d;

#[set(textures)]  // Must repeat for each constant
const sam: sampler;
```

### Texture Upload with COPY Command List
Texture upload requires a separate COPY command list:
```c
kore_gpu_command_list copy_list;
kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_COPY, &copy_list);

// Upload texture data via copy_list
kore_gpu_command_list_close(&copy_list);
kore_gpu_device_execute_command_list(&device, &copy_list);
```

### Sampler Parameters
```c
kore_gpu_sampler_parameters params = {
    .address_mode_u = KORE_GPU_ADDRESS_MODE_REPEAT,
    .address_mode_v = KORE_GPU_ADDRESS_MODE_REPEAT,
    .min_filter = KORE_GPU_FILTER_LINEAR,
    .mag_filter = KORE_GPU_FILTER_LINEAR,
    .mipmap_filter = KORE_GPU_MIPMAP_FILTER_LINEAR,
};
kore_gpu_device_create_sampler(&device, &params, &sampler);
```

### Convenience API Functions

New convenience functions added to simplify common tasks:

```c
// Create default sampler (linear filtering, repeat addressing, anisotropy 16)
kore_gpu_device_create_default_sampler(&device, &sampler);

// Create texture view with default parameters
kore_gpu_texture_view view;
kore_gpu_texture_view_create(&device, &texture, &view);

// One-step buffer upload (creates buffer + uploads data)
kore_gpu_buffer buffer;
kore_gpu_buffer_parameters params = { .size = size, .usage_flags = ... };
kore_gpu_device_create_buffer(&device, &params, &buffer);
kore_gpu_buffer_upload(&device, data, size, 0, &buffer);

// One-step texture upload (requires texture to be created with COPY_DST usage)
kore_gpu_texture_upload(&device, &texture, pixels, width, height);
```

### Matrix Math Helpers

New matrix functions added to `includes/kore3/math/matrix.h`:

```c
// Create perspective projection matrix
kore_matrix4x4 proj = kore_matrix4x4_perspective(fov, aspect, near, far);

// Create view matrix (camera)
kore_matrix4x4 view = kore_matrix4x4_look_at(
    (kore_float3){0, 0, 5},    // eye
    (kore_float3){0, 0, 0},    // center
    (kore_float3){0, 1, 0}     // up
);

// Combine matrices
kore_matrix4x4 mvp = kore_matrix4x4_multiply(&proj, &view);
```

### Vulkan Backend Status

| Feature | Status |
|---------|--------|
| Device creation | ✅ Complete |
| Buffer creation | ✅ Complete |
| Texture creation | ✅ Complete |
| Sampler creation | ✅ Complete |
| Descriptor sets | ✅ Complete |
| command_list_draw | ✅ Implemented |
| sampler_destroy | ✅ Implemented |
| clear_buffer | ✅ Implemented |
| Indirect drawing | ❌ Stub |
| Ray tracing | ❌ Stub |

## Metal vs Vulkan Matrix Conventions

### Coordinate Systems

| API | Coordinate System | Rotation Direction |
|-----|------------------|-------------------|
| **Metal** | Left-handed | Clockwise |
| **Vulkan** | Right-handed | Counter-clockwise |
| **OpenGL** | Right-handed | Counter-clockwise |

**Vulkan and Metal are different** - Vulkan uses right-handed coordinates (like OpenGL), while Metal uses left-handed coordinates.

### Implications

When writing cross-backend code, be aware that:
- Rotation direction is inverted between Metal and Vulkan
- Projection matrix may need adjustment for Z-axis
- Camera/View matrix transformations differ

### Code Example

```c
#if defined(KORE_METAL)
    // Metal: left-handed, clockwise rotation
    kore_matrix4x4 rotation = kore_matrix4x4_rotation_y(angle);
#elif defined(KORE_VULKAN) || defined(KORE_OPENGL)
    // Vulkan/OpenGL: right-handed, counter-clockwise rotation
    kore_matrix4x4 rotation = kore_matrix4x4_rotation_y(-angle);
#endif
```

### Metal Backend Configuration

The Metal backend is configured to use counter-clockwise winding to be consistent with OpenGL/Vulkan:

```objc
// In commandlist.m - begin_render_pass
[render_encoder setFrontFacingWinding:MTLWindingCounterClockwise];
[render_encoder setCullMode:MTLCullModeBack];
```
