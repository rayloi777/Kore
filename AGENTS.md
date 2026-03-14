# AGENTS.md - Kore3 Game Engine

**Generated:** 2026-03-11 | **Branch:** main

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
| Tests | `tests/cube_test/`, `tests/texture_test/`, `tests/audio_test/`, `tests/mipmap_test/`, `tests/computeshader_test/`, `tests/g2_test/`, `tests/raytracing_cornellbox/`, `tests/draw_test/` |

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

## TEXT RENDERING
- **Test:** `tests/draw_test/` - Font rendering with stb_truetype
- **Build:** `./make -g metal --kore . --from tests/draw_test --compile`

### API
```c
#include <kore3/text.h>

// Initialize drawing system
void draw_init(void *device, void *command_list);
void draw_set_viewport(int width, int height);

// Create/destroy fonts
draw_font *draw_font_create(const char *ttf_path, int *glyphs, int glyph_count, float size);
void draw_font_destroy(draw_font *font);

// Draw text (called within a render pass)
void draw_begin(void);
void draw_string(draw_font *font, const char *utf8_text, float x, float y, float r, float g, float b, float a);

// Get text width in pixels
float draw_string_width(draw_font *font, const char *utf8_text);
```

### Usage
```c
// Create multiple fonts with different sizes
draw_font *font_small = draw_font_create("NotoSansTC-Regular.ttf", kore_basic_glyphs, KORE_BASIC_GLYPHS_COUNT, 24.0f);
draw_font *font_medium = draw_font_create("NotoSansTC-Regular.ttf", kore_basic_glyphs, KORE_BASIC_GLYPHS_COUNT, 48.0f);
draw_font *font_large = draw_font_create("NotoSansTC-Regular.ttf", kore_basic_glyphs, KORE_BASIC_GLYPHS_COUNT, 72.0f);

// In render loop:
draw_begin();
if (font_small) draw_string(font_small, "Small text", 50.0f, 100.0f, 1.0f, 0.0f, 0.0f, 1.0f);  // red
if (font_medium) draw_string(font_medium, "Medium text", 50.0f, 200.0f, 0.0f, 1.0f, 0.0f, 1.0f);  // green
if (font_large) draw_string(font_large, "Large text", 50.0f, 350.0f, 0.0f, 0.0f, 1.0f, 1.0f);  // blue
```

### Implementation Details
- Each font creates its own texture atlas (512x512 RGBA8)
- Each font has its own descriptor set with texture bound at creation
- Uses stb_truetype for font rasterization
- Text rendered using immediate draw (each draw_string draws immediately)
- Per-vertex color support for different colored text
- Screen coordinates (0,0 = top-left) converted to NDC (-1 to 1)
- Y position includes font baseline adjustment

### Notes
- stb_truetype generates glyph bitmaps with proper vertical positioning in the bitmap
- The `yoffset` from stb_truetype is baked into the glyph bitmap position
- Baseline calculation: `(float)ascent * scale` - top of lowercase 'x' aligns here
- Descender characters (g, y, p, q, j) are positioned correctly in the atlas

## UI DIALOGS
- **Header:** `includes/kore3/ui.h`
- **Test:** `tests/ui_test/`
- **Build:** `./make -g metal --kore . --from tests/ui_test --compile`

### API
```c
#include <kore3/ui.h>

// Dialog types
KORE_UI_DIALOG_OK        // OK only
KORE_UI_DIALOG_OK_CANCEL // OK / Cancel
KORE_UI_DIALOG_YES_NO    // Yes / No

// Icon types
KORE_UI_DIALOG_INFO     // Information
KORE_UI_DIALOG_ERROR   // Error
KORE_UI_DIALOG_WARNING // Warning
KORE_UI_DIALOG_QUESTION // Question

// Show dialog - returns 1 for OK/Yes, 0 for Cancel/No
int kore_ui_dialog(const char *title, const char *message, int dialog_type, int icon);

// File chooser
typedef struct kore_ui_file_chooser_options {
    const char *title;
    const char *initial_directory;
    const char *file_name;
    const char **filters;   // e.g. {"Images", "png"}
    int filter_count;
    bool for_save;          // true = save dialog, false = open
    int window_id;         // parent window (0 for none)
} kore_ui_file_chooser_options;

#define KORE_UI_FILE_CHOOSER_OK     0
#define KORE_UI_FILE_CHOOSER_CANCEL 1

int kore_ui_file_chooser(kore_ui_file_chooser_options *options, char *buffer, int buffer_size);

// Clipboard
bool kore_ui_clipboard_set_text(const char *text);
char *kore_ui_clipboard_get_text(void);  // Caller must free() returned string
```

### Platform Support
| Feature | Windows | macOS | Linux |
|---------|---------|-------|-------|
| Dialog | ✅ | ✅ | ✅ |
| File Chooser | ✅ | ✅ | ✅ |
| Clipboard | ✅ | ✅ | ✅ |

### Implementation
- **Windows:** `backends/system/windows/sources/system.c` (MessageBoxW, GetOpenFileNameW)
- **macOS:** `backends/system/macos/sources/uiunit.m` (NSAlert, NSOpenPanel, NSPasteboard)
- **Linux:** `backends/system/linux/sources/ui.c` (GTK dialogs)
- **Other:** `sources/root/ui.c` (stub, logs warning)

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

## RAYTRACING (COMPUTE SHADER)
- **Test:** `tests/raytracing_cornellbox/` - Cornell Box scene with boxes and colored walls
- **Build:** `./make -g metal --kore . --from tests/raytracing_cornellbox --compile`

### Features
- Compute shader-based ray tracing (no Metal Ray Tracing API)
- Manual ray-box intersection for all geometry
- Cornell Box scene: tall box, short box, left wall (red), right wall (green), floor (gray), ceiling (white)
- Open back wall (viewable to sky)
- Thick walls (0.3 units) for proper geometry

### Controls
- **WASD**: Move camera horizontally
- **Q/E**: Move camera up/down
- **Mouse**: Look around (click window first to enable mouse capture)

### Implementation Details
- Uses compute shader with `#[compute, threads(16, 16, 1)]` dispatch
- Ray direction calculated from yaw/pitch angles passed via uniform buffer
- Box intersection: AABB slab method with separate tests for each object
- Scene bounds: -2.5 to 2.5 in X/Y/Z

### Historical Notes

The original implementation used Metal's Ray Tracing API (`MTLRayTracingPipelineDescriptor`, etc.), but this API was not available in the Xcode SDK (macOS 26.2). The API has since been stubbed out with `#if __has_include(<Metal/MTLRayTracing.h>)`.

**Kongruent Metal backend (`Kongruent/sources/backends/metal.c`):**
- Added ray pipeline support (`#[raypipe]` - raygen, raymiss, rayclosesthit, rayintersection, rayanyhit)
- Added rayset type detection for shader parameters
- Fixed `var_name()` to handle globals with sets_count == 0
- Fixed function signature generation for ray shaders (no descriptor buffer params)

**Kongruent integration (`Kongruent/sources/integrations/kore3.c`):**
- Fixed `kong_set_ray_pipeline_*` to use correct API
- Fixed Metal ray pipeline parameter names
