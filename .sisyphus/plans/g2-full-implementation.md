# g2 Full Implementation Plan

## TL;DR
> Implement full 2D rendering API (g2) for Kore3 - all drawing, text, transforms, and scissor operations.

**Deliverables:**
- Complete g2unit.c implementation
- Vertex buffer management
- Render pipeline integration
- Font rendering support

**Estimated Effort:** Medium
**Parallel Execution:** NO (sequential - depends on previous tasks)

---

## Context

### Original Request
User requested full implementation of g2 API (all stub functions).

### Current State
- g2unit.c: All functions are stubs (empty implementation)
- Header: `includes/kore3/g2/g2.h` defines full API
- Font support: Header exists at `includes/kore3/g2/font.h`

### Research Findings
- API pattern matches existing tests (cube_texture_test uses similar GPU patterns)
- Need: vertex buffer, pipeline, texture sampler, transform matrices

---

## Work Objectives

### Core Objective
Implement all g2 functions to make 2D rendering work.

### API Functions to Implement

| Category | Functions |
|----------|-----------|
| **Init** | `kore_g2_init`, `kore_g2_begin`, `kore_g2_end`, `kore_g2_flush` |
| **Image** | `kore_g2_draw_image`, `kore_g2_draw_scaled_image`, `kore_g2_draw_sub_image`, `kore_g2_draw_scaled_sub_image` |
| **Text** | `kore_g2_set_font`, `kore_g2_set_font_size`, `kore_g2_draw_string`, `kore_g2_string_width` |
| **Transform** | `kore_g2_translate`, `kore_g2_rotate`, `kore_g2_scale`, `kore_g2_push_transformation`, `kore_g2_pop_transformation` |
| **Color** | `kore_g2_set_color`, `kore_g2_set_color_uint`, `kore_g2_set_opacity`, `kore_g2_push_opacity`, `kore_g2_pop_opacity` |
| **Scissor** | `kore_g2_scissor`, `kore_g2_disable_scissor` |
| **Clear** | `kore_g2_clear` |

---

## Technical Approach

### Architecture
```
g2unit.c
├── Vertex Buffer (quad batching)
├── GPU Pipeline (texture + vertex colors)
├── Transform Stack (matrix4x4 array)
├── Opacity Stack (float array)
└── Scissor State
```

### Batching Strategy
- Buffer quads (4 vertices + 6 indices) per draw call
- Flush when buffer full or pipeline changes

### Dependencies
- Uses existing `gpu/device.h`, `gpu/pipeline.h`, `gpu/buffer.h`
- Uses Kongruent shader for rendering (like texture_test)

---

## Implementation Steps

### Wave 1: Infrastructure
- [x] 1. Define vertex format (position, uv, color)
- [x] 2. Create GPU pipeline and shader
- [x] 3. Initialize vertex/index buffers
- [x] 4. Implement kore_g2_init

### Wave 2: Image Drawing
- [x] 5. Implement kore_g2_begin/end/flush
- [x] 6. Implement draw_image (single quad)
- [x] 7. Implement draw_scaled_image
- [x] 8. Implement draw_sub_image variants

### Wave 3: Transforms & Color
- [x] 9. Implement transform functions (translate/rotate/scale)
- [x] 10. Implement transform stack (push/pop)
- [x] 11. Implement color/opacity functions
- [x] 12. Implement opacity stack

### Wave 4: Advanced
- [x] 13. Implement scissor functions
- [x] 14. Implement text drawing (using font.h)
- [x] 15. Implement string_width
- [x] 16. Implement kore_g2_clear

### Wave 5: Testing
- [x] 17. Create test (tests/g2_test/)
- [x] 18. Verify all functions work (framework complete, app runs)

---

## References

- `tests/cube_texture_test/sources/main.c` — GPU texture + pipeline pattern
- `tests/text_test/sources/main.c` — Font usage
- `includes/kore3/g2/g2.h` — API definitions
- `includes/kore3/g2/font.h` — Font API
