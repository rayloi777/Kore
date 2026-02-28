# Kongruent 著色器語法指南

Kongruent 是 Kore 的著色語言，用於編寫跨平台的 GPU 著色器。

## 目錄

1. [基本結構](#基本結構)
2. [頂點輸入/輸出](#頂點輸入輸出)
3. [統一變數 (Constants)](#統一變數-constants)
4. [紋理和採樣器](#紋理和採樣器)
5. [管線配置](#管線配置)
6. [C API 使用](#c-api-使用)

---

## 基本結構

```kong
struct vertex_in {
    // 頂點屬性
}

struct vertex_out {
    // 頂點着色器輸出
}

// 統一變數
#[set(everything)]
const constants: {
    // 均勻變量
};

// 頂點着色器
fun pos(input: vertex_in): vertex_out {
}

// 片段着色器
fun pix(input: vertex_out): float4 {
}

// 管線配置
#[pipe]
struct pipeline {
    vertex = pos;
    fragment = pix;
    // 選項
}
```

---

## 頂點輸入輸出

### 支援的類型

| Kongruent 類型 | C 類型 | 描述 |
|----------------|--------|------|
| `float` | `float` | 32 位浮點數 |
| `float2` | `kore_float2` | 2D 向量 |
| `float3` | `kore_float3` | 3D 向量 |
| `float4` | `kore_float4` | 4D 向量 / 顏色 |
| `float3x3` | `kore_matrix3x3` | 3x3 矩陣 |
| `float4x4` | `kore_matrix4x4` | 4x4 矩陣 |

### 示例

```kong
struct vertex_in {
    pos: float3;  // 位置
    uv: float2;    // UV 座標
    norm: float3; // 法線
    col: float3;  // 顏色
}

struct vertex_out {
    pos: float4;  // 裁剪空間位置
    uv: float2;   // 傳遞到片段着色器
    col: float3;  // 傳遞顏色
}
```

---

## 統一變數 (Constants)

使用 `#[set(everything)]` 區塊定義統一變數：

```kong
#[set(everything)]
const constants: {
    mvp: float4x4;      // MVP 矩陣
    time: float;       // 時間
    light_dir: float3; // 光照方向
};
```

### 帶索引的常量（用於紋理）

```kong
#[set(everything), indexed]
const constants: {
    mvp: float4x4;
};
```

---

## 紋理和採樣器

### 語法

```kong
#[set(everything)]
const tex: tex2d;     // 2D 紋理

#[set(everything)]
const sam: sampler;    // 採樣器
```

### 在片段着色器中使用

```kong
fun pix(input: vertex_out): float4 {
    var color: float4 = sample(tex, sam, input.uv);
    return color;
}
```

### 紋理類型

| 類型 | 描述 |
|------|------|
| `tex2d` | 2D 紋理 |
| `texcube` | 立方體紋理 |
| `tex3d` | 3D 紋理 |

---

## 管線配置

```kong
#[pipe]
struct pipeline {
    vertex = pos;           // 頂點着色器函數
    fragment = pix;         // 片段着色器函數
    
    // 深度/模板配置（可選）
    depth_stencil_format = TEXTURE_FORMAT_DEPTH32_FLOAT;
    depth_write = true;
    depth_compare = COMPARE_LESS;
    
    // 幀緩衝區格式
    format = framebuffer_format();
}
```

### 可用選項

| 選項 | 描述 |
|------|------|
| `vertex` | 頂點着色器函數名 |
| `fragment` | 片段着色器函數名 |
| `compute` | 計算着色器函數名 |
| `depth_stencil_format` | 深度格式 |
| `depth_write` | 是否寫入深度 |
| `depth_compare` | 深度比較函數 |
| `format` | 幀緩衝區格式 |

### 深度格式

- `TEXTURE_FORMAT_DEPTH32_FLOAT`
- `TEXTURE_FORMAT_DEPTH24_STENCIL8`

### 比較函數

- `COMPARE_LESS`
- `COMPARE_LEQUAL`
- `COMPARE_GREATER`
- `COMPARE_GEQUAL`
- `COMPARE_EQUAL`
- `COMPARE_NOT_EQUAL`
- `COMPARE_NEVER`
- `COMPARE_ALWAYS`

---

## C API 使用

### 1. 初始化

```c
#include <kong.h>

kore_gpu_device device;
kong_init(&device);
```

### 2. 創建緩衝區

```c
// 頂點緩衝區
vertex_in_buffer vertex_buf;
kong_create_buffer_vertex_in(&device, vertex_count, &vertex_buf);

// 均勻緩衝區
kore_gpu_buffer uniform_buffer;
constants_type_buffer_create(&device, &uniform_buffer, 1);
```

### 3. 創建描述符集

```c
everything_parameters params = {
    .constants = &uniform_buffer,
    // 紋理和採樣器（如果使用）
    // .tex = texture_view,
    // .sam = sampler,
};
everything_set set;
kong_create_everything_set(&device, &params, &set);
```

### 4. 渲染循環

```c
void render() {
    // 更新均勻緩衝區
    constants_type *ptr = constants_type_buffer_lock(&uniform_buffer, 0, 1);
    ptr->mvp = mvp_matrix;
    constants_type_buffer_unlock(&uniform_buffer);

    // 綁定管線
    kong_set_render_pipeline_pipeline(&list);

    // 綁定頂點緩衝區
    kong_set_vertex_buffer_vertex_in(&list, &vertex_buf);

    // 綁定描述符集
    kong_set_descriptor_set_everything(&list, &set, 0);

    // 繪製
    kore_gpu_command_list_draw_indexed(&list, index_count, 1, 0, 0, 0);
}
```

### 5. 清理

```c
kong_destroy_everything_set(&set);
constants_type_buffer_destroy(&uniform_buffer);
kong_destroy_buffer_vertex_in(&vertex_buf);
```

---

## 完整示例

### 著色器 (shaders.kong)

```kong
struct vertex_in {
    pos: float3;
    uv: float2;
}

struct vertex_out {
    pos: float4;
    uv: float2;
}

#[set(everything), indexed]
const constants: {
    mvp: float4x4;
};

fun pos(input: vertex_in): vertex_out {
    var output: vertex_out;
    output.pos = constants.mvp * float4(input.pos, 1.0);
    output.uv = input.uv;
    return output;
}

#[set(everything)]
const tex: tex2d;

#[set(everything)]
const sam: sampler;

fun pix(input: vertex_out): float4 {
    return sample(tex, sam, input.uv);
}

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

### C 代碼

```c
#include <kore3/gpu/device.h>
#include <kore3/system.h>
#include <kong.h>

static kore_gpu_device device;
static kore_gpu_command_list list;
static vertex_in_buffer vertex_buf;
static kore_gpu_buffer uniform_buffer;
static everything_set uniform_set;
static kore_gpu_texture depth_texture;

// 頂點數據
static vertex_in vertices[] = {
    { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f} },
};

static uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

int kickstart(int argc, char **argv) {
    kore_init("Texture Test", 1920, 1080, NULL, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);
    kong_init(&device);
    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

    // 創建頂點緩衝區
    kong_create_buffer_vertex_in(&device, 4, &vertex_buf);
    vertex_in *vtx = kong_vertex_in_buffer_lock(&vertex_buf);
    memcpy(vtx, vertices, sizeof(vertices));
    kong_vertex_in_buffer_unlock(&vertex_buf);

    // 創建均勻緩衝區
    constants_type_buffer_create(&device, &uniform_buffer, 1);

    // 創建深度紋理
    kore_gpu_texture_parameters depth_params = {
        .width = 1920, .height = 1080,
        .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format = KORE_GPU_TEXTURE_FORMAT_DEPTH32_FLOAT,
        .usage = KORE_GPU_TEXTURE_USAGE_RENDER_ATTACHMENT,
    };
    kore_gpu_device_create_texture(&device, &depth_params, &depth_texture);

    // 創建描述符集
    everything_parameters params = { .constants = &uniform_buffer };
    kong_create_everything_set(&device, &params, &uniform_set);

    kore_start();
    return 0;
}
```

---

## 常見問題

### Q: 如何添加多個紋理？

目前 Kongruent 僅支持一個紋理和一個採樣器。

### Q: 如何使用時間動畫？

```kong
#[set(everything)]
const constants: {
    time: float;
};

fun pos(input: vertex_in): vertex_out {
    // 使用 constants.time 進行動畫
}
```

### Q: 為什麼著色器崩潰？

確保 `#[pipe]` 區塊中的函數名稱與實際函數名稱匹配。

---

## 相關文件

- `tests/cube_test/shaders/shaders.kong` - 彩色立方體示例
- `tests/texture_test/shaders/shaders.kong` - 紋理示例
- `build/Kong-osx-metal/kong.h` - 生成的 C 頭文件
- `build/Kong-osx-metal/kong.metal` - 生成的 Metal 著色器
