# g2 Full Implementation Plan

## TL;DR
> Implement full 2D rendering API (g2) for Kore3 - all drawing, text, transforms, and scissor operations.

**Deliverables:**
- Complete g2unit.c implementation with GPU rendering
- Vertex buffer batching system
- GPU pipeline integration
- Font/text rendering support
- Modified g2_test to verify g2 API

**Estimated Effort:** Medium-High
**Parallel Execution:** NO (sequential - depends on previous tasks)

---

## Context

### Original Request
User requested full implementation of g2 API (all stub functions).

### Current State
- g2unit.c: All functions are stubs (empty implementation)
- Header: `includes/kore3/g2/g2.h` defines full API
- Font support: Header exists at `includes/kore3/g2/font.h`
- g2_test: Uses low-level Kongruent APIs, NOT g2 API

### Research Findings
- API pattern matches existing tests (cube_texture_test uses similar GPU patterns)
- Need: vertex buffer, pipeline, texture sampler, transform matrices
- Using GPU abstraction layer (backend-agnostic)

---

## Work Objectives

### Core Objective
Implement all g2 functions to make 2D rendering work.

### API Functions to Implement

| Category | Functions | Status |
|----------|-----------|--------|
| **Init** | `kore_g2_init`, `kore_g2_begin`, `kore_g2_end`, `kore_g2_flush` | TODO |
| **Image** | `kore_g2_draw_image`, `kore_g2_draw_scaled_image`, `kore_g2_draw_sub_image`, `kore_g2_draw_scaled_sub_image` | TODO |
| **Text** | `kore_g2_set_font`, `kore_g2_set_font_size`, `kore_g2_draw_string`, `kore_g2_string_width` | TODO |
| **Transform** | `kore_g2_translate`, `kore_g2_rotate`, `kore_g2_scale`, `kore_g2_push_transformation`, `kore_g2_pop_transformation` | TODO |
| **Color** | `kore_g2_set_color`, `kore_g2_set_color_uint`, `kore_g2_set_opacity`, `kore_g2_push_opacity`, `kore_g2_pop_opacity` | TODO |
| **Scissor** | `kore_g2_scissor`, `kore_g2_disable_scissor` | TODO |
| **Clear** | `kore_g2_clear` | TODO |

---

## Technical Approach

### Architecture
```
g2unit.c
├── Vertex Buffer (quad batching) - MAX_BUFFER_SIZE quads
├── GPU Pipeline (texture + vertex colors)
├── Transform Stack (matrix4x4 array, 32 levels)
├── Opacity Stack (float array, 32 levels)
└── Scissor State
```

### Vertex Format
```c
typedef struct {
    kore_vector2 pos;      // screen position
    kore_vector2 uv;         // texture coordinates
    kore_vector4 color;     // vertex color (RGBA)
} g2_vertex;
```

### Batching Strategy
- Buffer quads (4 vertices + 6 indices) per draw call
- Flush when buffer full or pipeline changes
- Use dynamic buffer with CPU write + GPU read

### GPU Pipeline Requirements
- Vertex shader: transform by MVP matrix
- Fragment shader: sample texture, multiply by vertex color
- Blending: alpha blending for transparency

---

## Implementation Steps

### Wave 1: Infrastructure
- [ ] 1. Define g2_vertex structure
- [ ] 2. Create vertex/index buffers in kore_g2_init
- [ ] 3. Set up orthographic projection matrix (screen → NDC)
- [ ] 4. Implement kore_g2_begin (store command list)
- [ ] 5. Implement kore_g2_flush (upload vertices, draw indexed)

### Wave 2: Basic Image Drawing
- [ ] 6. Implement kore_g2_draw_image (single quad at x,y with texture size)
- [ ] 7. Implement kore_g2_draw_scaled_image (specify dest dimensions)
- [ ] 8. Implement kore_g2_draw_sub_image (partial texture)
- [ ] 9. Implement kore_g2_draw_scaled_sub_image (partial + scaled)

### Wave 3: Transforms
- [ ] 10. Implement kore_g2_translate (modify transform matrix)
- [ ] 11. Implement kore_g2_rotate (around center point)
- [ ] 12. Implement kore_g2_scale
- [ ] 13. Implement kore_g2_push_transformation (save to stack)
- [ ] 14. Implement kore_g2_pop_transformation (restore from stack)

### Wave 4: Color & Opacity
- [ ] 15. Implement kore_g2_set_color (store in current color)
- [ ] 16. Implement kore_g2_set_color_uint (0xRRGGBBAA format)
- [ ] 17. Implement kore_g2_set_opacity (multiply into vertex color alpha)
- [ ] 18. Implement kore_g2_push_opacity
- [ ] 19. Implement kore_g2_pop_opacity

### Wave 5: Scissor & Clear
- [ ] 20. Implement kore_g2_scissor (set scissor rect)
- [ ] 21. Implement kore_g2_disable_scissor
- [ ] 22. Implement kore_g2_clear (clear render target)

### Wave 6: Text Rendering
- [ ] 23. Implement kore_g2_set_font
- [ ] 24. Implement kore_g2_set_font_size
- [ ] 25. Implement kore_g2_draw_string (use font.h API)
- [ ] 26. Implement kore_g2_string_width

### Wave 7: Integration & Testing
- [x] 27. Modify g2_test to use g2 API
- [ ] 28. Build and verify (compile + run)
- [ ] 29. Test all function categories

---

## Dependencies

### Internal
- `includes/kore3/gpu/device.h` — Device creation
- `includes/kore3/gpu/buffer.h` — Vertex/index buffers
- `includes/kore3/gpu/pipeline.h` — Render pipeline
- `includes/kore3/gpu/sampler.h` — Texture sampling
- `includes/kore3/gpu/texture.h` — Texture management
- `includes/kore3/math/matrix.h` — Matrix operations
- `includes/kore3/g2/font.h` — Font API

### External (Kongruent)
- Shader compilation via kong.h
- Generated pipeline types (kong_create_*, kong_set_*)

---

## References

- `tests/cube_texture_test/sources/main.c` — GPU texture + pipeline pattern
- `tests/text_test/sources/main.c` — Font usage
- `includes/kore3/g2/g2.h` — API definitions
- `includes/kore3/g2/font.h` — Font API
- `sources/2d/g2unit.c` — Current stub implementation
- `tests/g2_test/sources/main.c` — Test to modify

---

## Acceptance Criteria

1. All g2 functions in g2.h have non-stub implementations
2. g2_test can draw images using kore_g2_draw_image
3. Transforms (translate/rotate/scale) affect drawing position
4. Color/opacity modifications affect vertex colors
5. Font rendering works via kore_g2_draw_string
6. Scissor clips drawing to specified rectangle
7. Code compiles without errors on Metal backend

---

## Non-Goals

- graphics4/graphics5 compatibility (separate effort)
- Specific backend optimizations (Metal/Vulkan/OpenGL)
- Runtime font loading (use pre-baked fonts via font.h)
