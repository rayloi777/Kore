# G2 Text Rendering Fix Plan

## Problem
- Text "Hello" displays upside down (Y flipped)
- Root cause: Projection matrix translation values are in wrong positions

## Analysis

### Matrix Layout (Column-Major)
```
m[0] m[3] m[6]
m[1] m[4] m[7]
m[2] m[5] m[8]
```

### Current (WRONG)
```c
projection.m[2] = -1.0f;  // Translation in wrong position
projection.m[5] = -1.0f;  // Translation in wrong position
```

### Correct (with Y-flip)
```c
projection.m[6] = -1.0f;  // X translation (correct position)
projection.m[7] = 1.0f;   // Y translation (correct position, +1 for flip)
projection.m[4] = -2.0f / g_screen_height;  // Y scale negative = flip
```

## Tasks

- [ ] Fix projection matrix in `sources/2d/g2unit.c` `kore_g2_flush()` function
- [ ] Revert font UV flip in `sources/2d/fontunit.c` (if needed)
- [ ] Revert vertex Y swap in `sources/2d/g2unit.c` `kore_g2_draw_string()` (if needed)
- [ ] Build and test

## Files to Modify

1. `sources/2d/g2unit.c` - lines 160-169 (projection matrix)
2. `sources/2d/fontunit.c` - lines 253-256 (UV coordinates) 
3. `sources/2d/g2unit.c` - lines 447-452 (vertex Y coordinates)

## Expected Result
- Text renders correctly oriented (not upside down)
- Three colored rectangles still display correctly
