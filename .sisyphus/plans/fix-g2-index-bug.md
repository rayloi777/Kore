# Work Plan: Fix g2 Text Rendering - Index Buffer Bug

## Summary

**Task**: Fix index buffer generation bug in g2 text rendering  
**Root Cause**: Font loads correctly but nothing renders because index buffer creates 4 indices per quad instead of 6 (triangle list requires 6 indices per quad)  
**Scope**: Single file fix in sources/2d/g2unit.c

## Problem Analysis

### Verified Working
- Font loading: stbtt_BakeFontBitmap returns positive result
- Shader vertex format: Matches (36-byte stride: x,y,z,u,v,r,g,b,a)
- MVP and texture descriptors: Bound correctly in flush()
- Vertex buffer: Created with proper parameters

### Root Cause
In `sources/2d/g2unit.c` line ~139-142, the index generation creates sequential indices:
```c
for (int i = 0; i < g_vertex_count; i++) {
    indices[i] = i;  // Creates [0,1,2,3] for 4 vertices
}
```

This is wrong for triangle list rendering. A quad needs 6 indices (2 triangles):
- Triangle 1: vertices 0, 1, 2
- Triangle 2: vertices 0, 2, 3

## Implementation Steps

### Step 1: Fix Index Buffer Generation
**File**: sources/2d/g2unit.c  
**Location**: flush() function, lines ~139-146

**Change from**:
```c
uint16_t *indices = (uint16_t *)malloc(g_vertex_count * sizeof(uint16_t));
for (int i = 0; i < g_vertex_count; i++) {
    indices[i] = i;
}
```

**Change to**:
```c
int quad_count = g_vertex_count / 4;
int index_count = quad_count * 6;
uint16_t *indices = (uint16_t *)malloc(index_count * sizeof(uint16_t));
for (int q = 0; q < quad_count; q++) {
    int base = q * 4;
    int idx = q * 6;
    // Triangle 1: 0, 1, 2
    indices[idx + 0] = base + 0;
    indices[idx + 1] = base + 1;
    indices[idx + 2] = base + 2;
    // Triangle 2: 0, 2, 3
    indices[idx + 3] = base + 0;
    indices[idx + 4] = base + 2;
    indices[idx + 5] = base + 3;
}
```

### Step 2: Update Draw Call Index Count
**File**: sources/2d/g2unit.c  
**Location**: flush() function, line ~172

**Change from**:
```c
kore_gpu_command_list_draw_indexed(g_command_list, g_vertex_count, 1, 0, 0, 0);
```

**Change to**:
```c
kore_gpu_command_list_draw_indexed(g_command_list, index_count, 1, 0, 0, 0);
```

### Step 3: Update Index Buffer Size Allocation
**File**: sources/2d/g2unit.c  
**Location**: kore_g2_init() function, line ~105-109

**Change from**:
```c
kore_gpu_buffer_parameters index_params = {
    .size = MAX_INDICES * sizeof(uint16_t),
    .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
};
```

**Change to**:
```c
kore_gpu_buffer_parameters index_params = {
    .size = (MAX_BUFFER_SIZE * 6) * sizeof(uint16_t),  // 6 indices per quad
    .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
};
```

Also update MAX_INDICES define at top of file:
```c
#define MAX_INDICES (MAX_BUFFER_SIZE * 6)
```

## Verification Steps

1. **Build**: Compile text_test to verify no compile errors
2. **Run**: Execute text_test and verify text renders on screen
3. **Test**: Try different strings, colors, positions

## Risk Assessment

- **Risk Level**: Low
- **Reason**: Single file, isolated change, well-understood fix
- **Edge Cases**:
  - Vertex count not divisible by 4 (shouldn't happen with proper string rendering)
  - Empty string (handled by early return in draw_string)

## Success Criteria

- Text renders visibly on screen when running text_test
- No new compile errors introduced
- Font texture appears correctly (not required for this fix, but expected)
