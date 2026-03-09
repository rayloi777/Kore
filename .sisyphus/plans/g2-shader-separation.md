# G2 Text/Image Shader Separation Plan

## Problem
- G2 currently uses single shader for both images and text
- When switching between text and image rendering, GPU state issues occur
- Result: colored rectangles disappear, only text shows

## Solution
Create two separate shaders:
1. **g2_image_shader** - For colored rectangles/images
2. **g2_text_shader** - For text (R8 font texture with alpha from R channel)

---

## Tasks

### Phase 1: Create New Shaders

- [ ] **1.1** Create `tests/g2_test/shaders/image.kong`
  - Vertex: pos, uv, color
  - Fragment: texcolor * color (standard texture blending)

- [ ] **1.2** Modify `tests/g2_test/shaders/text.kong`
  - Use alpha from texture R channel
  - Premultiplied alpha blending

### Phase 2: Modify g2unit.c

- [ ] **2.1** Add pipeline tracking
  - Track current pipeline (image vs text)
  - Add `g_current_pipeline` variable

- [ ] **2.2** Add pipeline switching logic
  - In `kore_g2_draw_scaled_image()`: switch to image pipeline
  - In `kore_g2_draw_string()`: switch to text pipeline

- [ ] **2.3** Call correct pipeline before drawing
  - Flush before switching pipelines

### Phase 3: Build and Test

- [ ] **3.1** Build with new shaders
- [ ] **3.2** Test: colored rectangles should show
- [ ] **3.3** Test: text should show (may be upside down initially)

---

## Files to Modify

1. `tests/g2_test/shaders/image.kong` (NEW)
2. `tests/g2_test/shaders/text.kong` (NEW)
3. `sources/2d/g2unit.c` - Add pipeline switching

---

## Expected Result
- Three colored rectangles display correctly
- Text "Hello" displays correctly (may need Y-flip fix later)
- No more GPU state conflicts between text and image rendering
