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
| `tests/audio_test` | Audio playback (OGG + sine wave) | ✅ |
| `tests/matrix_test` | Matrix operations + SIMD benchmarks | ✅ |
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

### Build audio_test
```bash
./make -g metal --kore . --from tests/audio_test --compile
open build/build/Release/Audio-Test.app
```

### Build matrix_test
```bash
./make -g metal --kore . --from tests/matrix_test --compile
./build/build/Release/Matrix-Test.app/Contents/MacOS/Matrix-Test
```
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

## Metal & Vulkan Unified API

### NDC Z Range

Metal and Vulkan share the same NDC (Normalized Device Coordinates) Z range [0, 1]. This is unified in the codebase:

| API | NDC Z Range | Status |
|-----|-------------|--------|
| Metal | [0, 1] | ✅ Unified |
| Vulkan | [0, 1] | ✅ Unified |

### Common Header

The file `includes/kore3/gpu/common.h` provides shared definitions:

```c
#if defined(KORE_METAL) || defined(KORE_VULKAN)
    #define KORE_GPU_NDC_Z_ZERO_ONE 1
    #define KORE_GPU_FRAME_COUNT 2
    #define KORE_GPU_EXECUTION_FENCE_COUNT 8
    #define KORE_GPU_MAX_BUFFER_RANGES 16
#endif
```

### Perspective Matrix

Both Metal and Vulkan use the same perspective matrix formula:

```c
#if defined(KORE_METAL) || defined(KORE_VULKAN)
    kore_matrix4x4_set(&m, 2, 2, near / (near - far));
    kore_matrix4x4_set(&m, 2, 3, -1.0f);
    kore_matrix4x4_set(&m, 3, 2, near * far / (near - far));
#else
    // OpenGL: NDC Z ∈ [-1, 1]
    float nf = 1.0f / (near - far);
    kore_matrix4x4_set(&m, 2, 2, (far + near) * nf);
    kore_matrix4x4_set(&m, 2, 3, -1.0f);
    kore_matrix4x4_set(&m, 3, 2, 2 * far * near * nf);
#endif
```

### Viewport

- **Metal**: Standard viewport (origin at top-left)
- **Vulkan**: Y-axis flipped via negative viewport height

## Common Issues and Fixes

### texture_test Issues

#### MVP Matrix Caused Black Screen

**Problem**: texture_test showed black screen even though haxe.png was loaded.

**Root Cause**: Incorrect matrix multiplication order in MVP calculation.

**Original (WRONG)**:
```c
kore_matrix4x4 proj = kore_matrix4x4_perspective(fov, aspect, near, far);
kore_matrix4x4 view = kore_matrix4x4_look_at(...);
kore_matrix4x4 mvp = kore_matrix4x4_multiply(&view, &proj);  // WRONG
```

**Fix**: Use identity matrix for simple 2D rendering:
```c
kore_matrix4x4 mvp = kore_matrix4x4_identity();
```

Or use correct multiplication order:
```c
kore_matrix4x4 view_model = kore_matrix4x4_multiply(&model, &view);
kore_matrix4x4 mvp = kore_matrix4x4_multiply(&view_model, &proj);
```

#### Image File Not Found

**Problem**: `kore_image_init_from_file` returns 0 (file not found).

**Solution**: Ensure image file is in app bundle:
```javascript
// In kfile.js
project.addFile('deployment/**');
project.setDebugDir('deployment');
```
Then copy to app Resources or run from correct directory.

### Matrix Multiplication Order

**Problem**: `kore_matrix4x4_multiply(a, b)` computes `b * a` (reverse order).

**For standard MVP transformation** (proj * view * model):
```c
// WRONG - this computes proj * view
kore_matrix4x4 mvp = kore_matrix4x4_multiply(&proj, &view);

// CORRECT - multiply(a, b) = b * a
kore_matrix4x4 model = ...;
kore_matrix4x4 view = ...;
kore_matrix4x4 proj = ...;
kore_matrix4x4 view_model = kore_matrix4x4_multiply(&model, &view);  // = view * model
kore_matrix4x4 mvp = kore_matrix4x4_multiply(&view_model, &proj);    // = proj * view * model
```

**For simple 2D rendering** (identity matrix):
```c
kore_matrix4x4 mvp = kore_matrix4x4_identity();
```

### Kong API Vertex Buffer Types

**Problem**: Using wrong buffer type causes crashes.

```c
// WRONG - kore_gpu_buffer is generic
static kore_gpu_buffer vertex_buffer;

// CORRECT - use Kong generated types
static vertex_in_buffer vertex_buffer;

// WRONG - manual lock/unlock
void *ptr = kore_gpu_buffer_lock_all(&vertex_buffer);

// CORRECT - use Kong helper functions
vertex_in *ptr = kong_vertex_in_buffer_lock(&vertex_buffer);
memcpy(ptr, vertices, sizeof(vertices));
kong_vertex_in_buffer_unlock(&vertex_buffer);
```

### Texture Image Loading Path

**Problem**: Image file not found at runtime.

The image path is relative to the app's working directory. For Metal apps, ensure the image is in the app bundle:
```bash
# Copy image to app Resources
cp tests/texture_test/deployment/haxe.png build/build/Release/Texture-Test.app/Contents/Resources/
```

Or in kfile.js, ensure deployment folder is included:
```javascript
project.addFile('deployment/**');
project.setDebugDir('deployment');
```

Then load with relative path:
```c
size_t image_size = kore_image_init_from_file(&image, memory, "haxe.png");
```

### Perspective Matrix NDC Z Range

**Problem**: Metal and Vulkan use NDC Z range [0, 1], while OpenGL uses [-1, 1].

```c
kore_matrix4x4 kore_matrix4x4_perspective(float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov / 2.0f);
    
    kore_matrix4x4 m = {0};
    kore_matrix4x4_set(&m, 0, 0, f / aspect);
    kore_matrix4x4_set(&m, 1, 1, f);
    
#if defined(KORE_METAL) || defined(KORE_VULKAN)
    // Metal & Vulkan: NDC Z ∈ [0, 1]
    kore_matrix4x4_set(&m, 2, 2, near / (near - far));
    kore_matrix4x4_set(&m, 2, 3, -1.0f);
    kore_matrix4x4_set(&m, 3, 2, near * far / (near - far));
#else
    // OpenGL: NDC Z ∈ [-1, 1]
    float nf = 1.0f / (near - far);
    kore_matrix4x4_set(&m, 2, 2, (far + near) * nf);
    kore_matrix4x4_set(&m, 2, 3, -1.0f);
    kore_matrix4x4_set(&m, 3, 2, 2 * far * near * nf);
#endif
    return m;
}

## Audio API

Kore provides two levels of audio APIs.

### Mixer API (High-level - Recommended)

Best for playing audio files with automatic mixing.

#### Playing Sound Effects (WAV/OGG)

```c
#include <kore3/mixer/mixer.h>
#include <kore3/mixer/sound.h>

// Initialize
kore_mixer_init();

// Create sound (pre-decoded, good for short effects)
kore_mixer_sound *sound = kore_mixer_sound_create("sound.wav");
// Or from memory
// kore_mixer_sound *sound = kore_mixer_sound_create_from_buffer(data, size, KORE_MIXER_AUDIOFORMAT_WAV);

// Set volume
kore_mixer_sound_set_volume(sound, 0.8f);

// Play
kore_mixer_channel *channel = kore_mixer_play_sound(sound, false, 1.0f, false);
// Parameters: sound, loop, pitch, unique

// Control volume while playing
kore_mixer_channel_set_volume(channel, 0.5f);

// Stop
kore_mixer_stop_sound(sound);

// Destroy
kore_mixer_sound_destroy(sound);
```

#### Playing Music (Streaming)

```c
#include <kore3/mixer/soundstream.h>

// Create stream (decoded while playing, good for long audio)
kore_mixer_sound_stream *stream = kore_mixer_sound_stream_create("music.ogg", true);
// Parameters: filename, looping

// Set volume
kore_mixer_sound_stream_set_volume(stream, 0.5f);

// Play
kore_mixer_play_sound_stream(stream);

// Control
kore_mixer_sound_stream_set_looping(stream, true);
float pos = kore_mixer_sound_stream_position(stream);  // Current position (seconds)
float len = kore_mixer_sound_stream_length(stream);    // Total length (seconds)

// Stop
kore_mixer_stop_sound_stream(stream);

// Reset to beginning
kore_mixer_sound_stream_reset(stream);
```

#### Main Loop

```c
void update(void *data) {
    kore_audio_update();  // Call every frame
    // ...
}
```

### Audio API (Low-level)

For custom audio generation, directly provide audio samples.

```c
#include <kore3/audio/audio.h>

void audio_callback(kore_audio_buffer *buffer, uint32_t samples, void *userdata) {
    // Write samples directly to buffer->channels[0] (left) and buffer->channels[1] (right)
    for (uint32_t i = 0; i < samples; i++) {
        float sample = generate_sample();  // Custom generation
        buffer->channels[0][buffer->write_location] = sample;
        buffer->channels[1][buffer->write_location] = sample;
        buffer->write_location = (buffer->write_location + 1) % buffer->data_size;
    }
}

// Initialize
kore_audio_init();
kore_audio_set_callback(audio_callback, NULL);

// Get sample rate
uint32_t rate = kore_audio_samples_per_second();  // Usually 44100 or 48000

// Cleanup
kore_audio_shutdown();
```

### API Selection

| API | Use Case | Features |
|-----|----------|----------|
| **Mixer** | Play audio files | High-level, auto-mixing, WAV/OGG support |
| **Audio** | Custom audio generation | Low-level, direct sample control |

### File Types

| Type | Description |
|------|-------------|
| `kore_mixer_sound` | Sound effect (pre-decoded) |
| `kore_mixer_sound_stream` | Stream (decoded while playing) |

### Example: audio_test

See `tests/audio_test/sources/main.c` for a complete example including:
- OGG music playback via `kore_mixer_sound_stream`
- Custom sine wave generation via `kore_audio_set_callback`
- Play/Pause/Stop controls
- Loop toggle and position tracking

**Controls:**
| Key | Function |
|-----|----------|
| `P` | Play/Pause music |
| `S` | Stop music (reset to beginning) |
| `L` | Toggle loop |
| `I` | Show music info |
| `1` | Toggle sine wave mode |
| `2/3` | Increase/Decrease frequency |

## SIMD Matrix Operations

Kore provides SIMD-optimized matrix operations for improved performance.

### SIMD Functions

```c
#include <kore3/math/matrix.h>

// SIMD optimized versions (4-8x faster than scalar)
kore_matrix4x4 kore_matrix4x4_multiply_simd(kore_matrix4x4 *a, kore_matrix4x4 *b);
kore_float4    kore_matrix4x4_multiply_vector_simd(kore_matrix4x4 *a, kore_float4 b);
void           kore_matrix4x4_transpose_simd(kore_matrix4x4 *matrix);
```

### Performance Benchmarks

| Operation | Speedup |
|-----------|---------|
| Matrix Multiply (4x4) | ~8.4x |
| Matrix-Vector Multiply | ~8.1x |
| Matrix Transpose | ~2.2x |

### Example: matrix_test

See `tests/matrix_test/sources/main.c` for a complete example including:
- Correctness tests comparing SIMD vs scalar results
- Performance benchmarks with timing
- All matrix operations (identity, rotation, translation, scale, multiply, transpose)

**Build and Run:**
```bash
./make -g metal --kore . --from tests/matrix_test --compile
./build/build/Release/Matrix-Test.app/Contents/MacOS/Matrix-Test
```

**Expected Output:**
```
=== Matrix Library Tests ===
...
=== SIMD Correctness Tests ===
Testing kore_matrix4x4_multiply_simd() correctness...
  PASSED
...
=== Performance Benchmarks ===
=== Matrix Multiply Benchmark (1000000 iterations) ===
  Speedup: 8.39x
...
ALL TESTS PASSED!
```
