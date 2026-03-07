# Kfile.js Shader 流程深度分析

**分析日期:** 2026-03-07 | **範圍:** Kore3 Kongruent 著色器系統

---

## 1. 總覽架構

Kore3 使用 **Kongruent** 作為跨平台著色器語言，配合 **kfile.js** 構建系統。

```
.kong (著色器源碼)
    ↓
kongruent 編譯器
    ↓
kong.m + kong.h (平台特定代碼)
    ↓
C 代碼調用 kong_* 函數
```

---

## 2. Kfile.js 配置

### 2.1 根 Kfile.js

文件位置: `/kore3/Kore3/kfile.js:520`

- 定義整個引擎的項目配置
- 根據平台和圖形 API 添加後端
- 不直接處理著色器

### 2.2 測試項目 Kfile.js

典型配置 (`tests/cube_test/kfile.js:14`):

```javascript
let project = new Project('Cube-Test');
await project.addProject(findKore());
project.addFile('sources/**');
project.addKongDir('shaders');           // ← 添加 Kongruent 著色器目錄
project.setDebugDir('deployment');

project.addIncludeDir('../../build/Kong-osx-metal');
project.addFile('../../build/Kong-osx-metal/kong.m');  // ← 引用編譯後的著色器

project.flatten();
resolve(project);
```

### 2.3 關鍵方法

| 方法 | 用途 |
|------|------|
| `project.addKongDir('shaders')` | 添加 Kongruent 著色器目錄，觸發編譯 |
| `project.addFile('../../build/Kong-*/kong.m')` | 引用編譯後的著色器代碼 |
| `project.flatten()` | 扁平化項目結構 |

---

## 3. Kongruent 著色器語法

### 3.1 文件結構

示例: `tests/cube_test/shaders/shaders.kong:45`

```kong
struct vertex_in {
    pos: float3;     // 頂點位置
    col: float3;     // 顏色
}

struct vertex_out {
    pos: float4;     // 裁剪空間位置
    col: float3;     // 傳遞到片段著色器
}

// 統一變數 (Uniforms)
#[set(everything)]
const constants: {
    mvp: float4x4;   // MVP 矩陣
};

// 頂點著色器
fun pos(input: vertex_in): vertex_out {
    var output: vertex_out;
    output.pos = constants.mvp * float4(input.pos, 1.0);
    output.col = input.col;
    return output;
}

// 片段著色器
fun pix(input: vertex_out): float4 {
    return float4(input.col, 1.0);
}

// 管線配置
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

### 3.2 支持的類型

| Kongruent 類型 | C 類型 | 描述 |
|----------------|--------|------|
| `float` | `float` | 32 位浮點數 |
| `float2` | `kore_float2` | 2D 向量 |
| `float3` | `kore_float3` | 3D 向量 |
| `float4` | `kore_float4` | 4D 向量 / 顏色 |
| `float3x3` | `kore_matrix3x3` | 3x3 矩陣 |
| `float4x4` | `kore_matrix4x4` | 4x4 矩陣 |

### 3.3 紋理和採樣器

```kong
#[set(everything)]
const tex: tex2d;

#[set(everything)]
const sam: sampler;

fun pix(input: vertex_out): float4 {
    return sample(tex, sam, input.uv);
}
```

---

## 4. 編譯流程

### 4.1 Build 命令

```bash
./make -g metal --kore . --from tests/cube_test --compile
```

### 4.2 kmake 內部流程

```
kfile.js 解析
    ↓
addKongDir('shaders') 觸發 kongruent 編譯
    ↓
tools/macos_arm64/kongruent 編譯 .kong 文件
    ↓
生成:
  - build/Kong-osx-metal/kong.m    (Metal 著色器代碼)
  - build/Kong-osx-metal/kong.h    (類型定義)
    ↓
Xcode 項目生成 + 編譯
```

### 4.3 工具位置

| 工具 | 位置 |
|------|------|
| kmake | `tools/macos_arm64/kmake` |
| kongruent | `tools/macos_arm64/kongruent` |

---

## 5. Runtime 使用 (C 代碼)

### 5.1 初始化

示例: `tests/cube_test/sources/main.c:170-225`

```c
#include <kong.h>

static kore_gpu_device device;
static vertex_in_buffer vertex_buffer;
static kore_gpu_buffer uniform_buffer;
static everything_set uniform_set;

int kickstart(int argc, char **argv) {
    kore_gpu_device_create(&device, &wishlist);
    kong_init(&device);  // ← 初始化 Kong
    
    // 創建頂點緩衝區
    kong_create_buffer_vertex_in(&device, 24, &vertex_buffer);
    vertex_in *ptr = kong_vertex_in_buffer_lock(&vertex_buffer);
    memcpy(ptr, vertices, sizeof(vertices));
    kong_vertex_in_buffer_unlock(&vertex_buffer);
    
    // 創建 Uniform 緩衝區
    constants_type_buffer_create(&device, &uniform_buffer, 1);
    
    // 創建描述符集
    everything_parameters params = { .constants = &uniform_buffer };
    kong_create_everything_set(&device, &params, &uniform_set);
}
```

### 5.2 渲染循環

```c
void update(void *data) {
    // 更新 Uniform
    constants_type *ptr = constants_type_buffer_lock(&uniform_buffer, 0, 1);
    *ptr = uniforms;
    constants_type_buffer_unlock(&uniform_buffer);
    
    // 綁定管線
    kong_set_render_pipeline_pipeline(&list);
    
    // 綁定頂點緩衝區
    kong_set_vertex_buffer_vertex_in(&list, &vertex_buffer);
    
    // 綁定描述符集
    kong_set_descriptor_set_everything(&list, &uniform_set);
    
    // 繪製
    kore_gpu_command_list_draw_indexed(&list, 36, 1, 0, 0, 0);
}
```

---

## 6. 關鍵函數映射

| Kong 函數 | 用途 |
|-----------|------|
| `kong_init(&device)` | 初始化著色器系統 |
| `kong_create_buffer_vertex_in()` | 創建頂點緩衝區 |
| `kong_vertex_in_buffer_lock/unlock` | 訪問頂點數據 |
| `constants_type_buffer_create` | 創建 Uniform 緩衝區 |
| `constants_type_buffer_lock/unlock` | 更新 Uniform 數據 |
| `kong_create_everything_set` | 創建描述符集 |
| `kong_set_render_pipeline_pipeline` | 綁定渲染管線 |
| `kong_set_vertex_buffer_vertex_in` | 綁定頂點緩衝區 |
| `kong_set_descriptor_set_everything` | 綁定 Uniform/紋理 |

---

## 7. 數據流總圖

```
┌─────────────────────────────────────────────────────────────┐
│                    Kfile.js 配置                            │
│  addKongDir('shaders') → addFile('build/.../kong.m')       │
└─────────────────────────┬───────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────────┐
│                   kmake 構建系統                             │
│  1. 解析 kfile.js                                           │
│  2. 調用 kongruent 編譯器                                    │
│  3. 生成 kong.m (Metal) / kong.h                            │
└─────────────────────────┬───────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────────┐
│                  .kong 源文件                                │
│  struct vertex_in / vertex_out                              │
│  fun pos() → vertex shader                                 │
│  fun pix() → fragment shader                               │
│  #[pipe] pipeline config                                   │
└─────────────────────────┬───────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────────┐
│                 生成的 kong.m/h                              │
│  - vertex_in_buffer 結構體                                 │
│  - constants_type 結構體                                      │
│  - everything_set 描述符集                                   │
│  - kong_set_* 綁定函數                                        │
└─────────────────────────┬───────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────────┐
│                    C 運行時代碼                              │
│  kong_init() → kong_create_*() → kong_set_*() → draw()   │
└─────────────────────────────────────────────────────────────┘
```

---

## 8. 支持的圖形 API

| 平台 | 默認 API | 生成的著色器 |
|------|----------|-------------|
| macOS | Metal | `kong.metal` |
| iOS/tvOS | Metal | `kong.metal` |
| Windows | Direct3D12 | `kong.hlsl` |
| Linux | Vulkan | `kong.spv` (SPIR-V) |
| Android | Vulkan | `kong.spv` |
| Web | WebGL/OpenGL | `kong.glsl` |

---

## 9. 相關文件索引

| 文件 | 用途 |
|------|------|
| `kfile.js` | 項目構建配置 |
| `tests/cube_test/kfile.js` | 測試項目配置示例 |
| `tests/cube_test/shaders/shaders.kong` | 著色器源碼示例 |
| `tests/cube_test/sources/main.c` | C 運行時代碼示例 |
| `kongruent_help.md` | Kongruent 語法文檔 |
| `kmake_help.md` | kmake 使用指南 |
| `kong.h` | Kong 集成頭文件 |
| `kong.c` | Kong 集成實現 |

---

## 10. 總結

**Kfile.js 的 Shader 流程**:

1. **配置階段**: `addKongDir()` 指定著色器目錄
2. **編譯階段**: `kongruent` 將 `.kong` 轉換為平台代碼
3. **鏈接階段**: `addFile('build/.../kong.m')` 將生成的代碼加入項目
4. **運行階段**: C 代碼通過 `kong_*` 函數使用著色器

**設計優勢**:

- **跨平台**: 同一套 `.kong` 源碼編譯到不同 GPU API
- **類型安全**: 生成的 C 結構體確保編譯時檢查
- **Developer UX**: 簡化的 API (`kong_set_*`) 隱藏底層複雜性

---

## 11. Matrix 問題修復 (2026-03-08)

### 11.1 問題描述

使用 `kore_matrix4x4_perspective()` 時，物體不可見或變形。

### 11.2 參考來源

參考 Kha 官方範例: https://github.com/Kode/Kore-Samples/blob/main/04_textured_cube/sources/main.c

### 11.3 Perspective Matrix (OpenGL 格式)

Kha 使用的矩陣格式，與 OpenGL 相同:

```c
kore_matrix4x4 kore_matrix4x4_perspective(float fov, float aspect, float near, float far) {
    float uh = 1.0f / tanf(fov / 2.0f);
    float uw = uh / aspect;
    
    kore_matrix4x4 m = {
        .m = {
            uw, 0, 0, 0,
            0, uh, 0, 0,
            0, 0, (far + near) / (near - far), -1,
            0, 0, (2 * far * near) / (near - far), 0
        }
    };
    return m;
}
```

### 11.4 MVP 矩陣順序

```c
kore_matrix4x4 proj = kore_matrix4x4_perspective(fov, aspect, 0.1f, 100.0f);
kore_matrix4x4 view = kore_matrix4x4_look_at(eye, center, up);
kore_matrix4x4 model = kore_matrix4x4_identity();

kore_matrix4x4 mvp = model;
mvp = kore_matrix4x4_multiply(&mvp, &view);   // M * V
mvp = kore_matrix4x4_multiply(&mvp, &proj);   // (M * V) * P
```

### 11.5 LookAt 函數

```c
kore_matrix4x4 kore_matrix4x4_look_at(kore_float3 eye, kore_float3 at, kore_float3 up) {
    kore_float3 zaxis = kore_float3_normalize(kore_float3_sub(at, eye));
    kore_float3 xaxis = kore_float3_normalize(kore_float3_cross(zaxis, up));
    kore_float3 yaxis = kore_float3_cross(xaxis, zaxis);
    
    kore_matrix4x4 m = {
        xaxis.x, yaxis.x, -zaxis.x, 0,
        xaxis.y, yaxis.y, -zaxis.y, 0,
        xaxis.z, yaxis.z, -zaxis.z, 0,
        -kore_float3_dot(xaxis, eye), -kore_float3_dot(yaxis, eye), kore_float3_dot(zaxis, eye), 1
    };
    return m;
}
```

### 11.6 使用範例

**Triangle test:**
```c
kore_float3 eye = {0, 0, 4};
kore_float3 center = {0, 0, 0};
kore_float3 up = {0, 1, 0};

kore_matrix4x4 proj = kore_matrix4x4_perspective(fov, aspect, 0.001f, 1000.0f);
kore_matrix4x4 view = kore_matrix4x4_look_at(eye, center, up);
kore_matrix4x4 model = kore_matrix4x4_identity();
```

**Cube test:**
```c
kore_float3 eye = {4, 3, 3};
kore_float3 center = {0, 0, 0};
kore_float3 up = {0, 1, 0};
```

### 11.7 關鍵點

| 元素 | 值 | 說明 |
|------|-----|------|
| m[0][0] | uw | 水平縮放 |
| m[1][1] | uh | 垂直縮放 |
| m[2][2] | (far+near)/(near-far) | Z 映射 |
| m[2][3] | -1 | OpenGL Z 方向 |
| m[3][2] | 2*far*near/(near-far) | Z 偏移 |

---

*Generated by Prometheus Analysis Mode*
