# Kore 3 專案資訊

## 專案簡介

Kore 3 是一個用於跨平台遊戲引擎開發的低階工具包，類似 SDL 但範圍更廣。主要特點包括：

- **跨平台支援**：Windows、Linux、macOS、iOS、Android、FreeBSD
- **多種 GPU API**：Direct3D 11/12、OpenGL、Vulkan、Metal、WebGPU、軟體渲染 (Kompjuta)
- **自訂著色語言**：Kongruent
- **建置系統**：kmake（元建置工具）

這是 Kore 的最新版本，採用新的 Graphics API（類似 WebGPU）和自訂著色器語言。

## 系統架構

```
┌─────────────────────────────────────────────────────────┐
│                    應用層 (Tests/Projects)               │
└─────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────┐
│                   公開 API (includes/kore3/)             │
│  ┌────────┬────────┬────────┬────────┬────────┬───────┐ │
│  │  gpu   │  audio │ input  │  math  │ 2d/    │system │ │
│  │        │        │        │        │video   │window │ │
│  └────────┴────────┴────────┴────────┴────────┴───────┘ │
└─────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
┌───────────────┐   ┌───────────────┐   ┌───────────────┐
│  GPU Backend  │   │ Audio Backend │   │System Backend │
│  (backends/gpu/)   │(backends/audio/)  │(backends/system/)
│  ┌─────────┐  │   │               │   │               │
│  │ Metal   │  │   │               │   │               │
│  │ OpenGL  │  │   │               │   │               │
│  │ Vulkan  │  │   │               │   │               │
│  │ D3D11   │  │   └───────────────┘   └───────────────┘
│  │ D3D12   │  │
│  │ WebGPU  │  │
│  │ Kompjuta│  │
│  └─────────┘  │
└───────────────┘
        │
┌─────────────────────────────────────────────────────────┐
│              實現層 (sources/)                           │
│  2d/ audio/ gpu/ input/ io/ math/ network/ util/ ...   │
└─────────────────────────────────────────────────────────┘
```

### 核心模組

| 目錄 | 說明 |
|------|------|
| `gpu/` | 新 Graphics API（WebGPU 風格）- 這是 Kore 3 的核心 |
| `audio/` | 音訊處理 |
| `input/` | 輸入系統（鍵盤、滑鼠、手柄） |
| `math/` | 數學庫（向量、矩陣等） |
| `2d/` | 2D 渲染 |
| `system/` | 系統級功能（視窗、執行緒等） |

### GPU 後端支援

| 後端 | 平台 | 狀態 |
|------|------|------|
| Metal | macOS/iOS | ✅ 正常運作 |
| OpenGL | 跨平台 | ✅ 正常運作 |
| Vulkan | 跨平台 | ✅ 正常運作 |
| Direct3D 11 | Windows | ✅ 正常運作 |
| Direct3D 12 | Windows | ✅ 正常運作 |
| WebGPU | Web | ✅ 正常運作 |
| Kompjuta | 軟體渲染 | ✅ 正常運作 |

### WebGPU 說明

**重要：Kore 的 WebGPU 後端只能在 WebAssembly (Emscripten) 環境下運行**，不是 macOS 原生。

```c
#ifdef KORE_EMSCRIPTEN
    #include <webgpu/webgpu.h>   // 真正的 WebGPU 實現
#else
    // 空 stub，什麼都不做
#endif
```

| 環境 | 使用的 GPU 後端 |
|------|----------------|
| **Web 瀏覽器** (WASM) | WebGPU (透過 Emscripten) |
| **macOS 原生** | Metal |
| **Windows 原生** | Direct3D 11/12, Vulkan, OpenGL |
| **Linux 原生** | Vulkan, OpenGL |
| **iOS 原生** | Metal |
| **Android 原生** | Vulkan, OpenGL ES |

### WebGPU 瀏覽器支援

WebGPU 已於 2025 年 11 月達到主流瀏覽器全面支援。

| 平台 | 瀏覽器 | 支援狀態 |
|------|--------|----------|
| **iOS** | Safari | ✅ iOS 26+ 支援 |
| **Android** | Chrome | ✅ 支援 |
| **Desktop** | Chrome, Firefox, Safari, Edge | ✅ 全面支援 |

### WebGPU 原生 App 支援

WebGPU 主要是為 **Web 平台**設計的，原生 App 沒有官方支援：

| 平台 | 官方 WebGPU 支援 | 第三方綁定 |
|------|-----------------|------------|
| **Swift 原生 App (iOS)** | ❌ 無官方支援 | 實驗性綁定 |
| **Kotlin 原生 App (Android)** | ❌ 無官方支援 | wgpu4k (實驗性) |

**建議：**
- iOS 原生 App → 使用 **Metal**
- Android 原生 App → 使用 **Vulkan** 或 **OpenGL ES**

Kore 已經支援這些原生 API。

## 建置說明

### 前置作業

```bash
# 初始化 git submodules（下載 kmake 建置工具）
./get_dlc

# 下載 Metal 工具鏈（用於著色器編譯）
xcodebuild -downloadComponent MetalToolchain
```

### 建置測試

```bash
# 使用 Metal 後端建置
./make -g metal --kore . --from tests/empty --compile

# 使用 Metal 後端建置 cube_test
./make -g metal --kore . --from tests/cube_test --compile

# 使用 OpenGL 後端建置
./make -g opengl --kore . --from tests/empty --compile

# 使用 WebGPU 後端建置（Web 應用）
./make -g webgpu --kore . --from tests/empty --compile

# 執行建置的應用程式
open build/build/Release/<TestName>.app
```

### 可用測試

| 測試 | 說明 | 狀態 |
|------|------|------|
| `tests/empty` | 最小視窗 | ✅ |
| `tests/shader-gpu` | 著色器測試 | ✅ |
| `tests/simd` | SIMD 運算 | ✅ |
| `tests/cube_test` | 旋轉立方體（帶顏色、深度測試） | ✅ |
| `tests/shader-g5` | 需要 graphics5 API | ❌ 未實作 |
| `tests/shader` | 需要 graphics4 API | ❌ 未實作 |

### 程式碼格式化

```bash
# 格式化單一檔案
clang-format -style=file -i <file>

# 格式化整個程式碼庫
node ./.github/format.js
```

## 編譯系統架構

Kore 使用 kmake（元建置工具）實現跨平台編譯。

```
┌─────────────────────────────────────────────────────────┐
│                  ./make                                  │
│                  (包裝腳本)                               │
└─────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│                  kmake                                  │
│     (Kore Meta Build Tool - 位於 tools/)                │
│  - 讀取 kfile.js 配置文件                              │
│  - 根據 platform/graphics 參數生成 Xcode/MSVC 專案     │
└─────────────────────────────────────────────────────────┘
                            │
        ┌───────────┬───────────┬───────────┬───────────┐
        ▼           ▼           ▼           ▼           ▼
    macOS       iOS       Android     Windows     Linux
   (osx)       (ios)     (android)   (windows)  (linux)
```

### 編譯參數

```bash
./make -g <graphics> -t <platform> --from <project> --compile
```

| 參數 | 說明 | 可選值 |
|------|------|--------|
| `-g` | Graphics API | `metal`, `opengl`, `vulkan`, `direct3d11`, `direct3d12`, `webgpu` |
| `-t` | Target platform | `osx`, `ios`, `android`, `windows`, `linux` |

### 平台對照表

| 平台 | 系統後端 | GPU 後端 | 預設圖形 API |
|------|---------|---------|--------------|
| macOS | system/apple, system/macos, system/posix | Metal/OpenGL | Metal |
| iOS | system/apple, system/ios, system/posix | Metal/OpenGL ES | Metal |
| Android | system/android, system/posix | Vulkan/OpenGL ES | Vulkan |
| Windows | system/windows, system/microsoft | D3D11/D3D12/OpenGL/Vulkan | D3D12 |
| Linux | system/linux, system/posix | Vulkan/OpenGL | Vulkan |
| Web (Emscripten) | system/emscripten | WebGPU | - |

### kfile.js 條件編譯

根據 `platform` 和 `graphics` 參數，kfile.js 選擇性加入後端：

```javascript
// iOS + Metal
else if (platform === Platform.iOS) {
    addBackend('system/apple');
    addBackend('system/ios');
    addBackend('system/posix');
    if (graphics === GraphicsApi.Metal || graphics === GraphicsApi.Default) {
        addBackend('gpu/metal');
        addKoreDefine('METAL');
        project.addLib('Metal');
    }
    // ...
}

// Android + Vulkan
else if (platform === Platform.Android) {
    addBackend('system/android');
    addBackend('system/posix');
    if (graphics === GraphicsApi.Vulkan || graphics === GraphicsApi.Default) {
        addBackend('gpu/vulkan');
        addKoreDefine('VULKAN');
        // ...
    }
    // ...
}
```

### 後端選擇機制

```javascript
function addBackend(name) {
    project.addIncludeDir('backends/' + name + '/includes');
    project.addFile('backends/' + name + '/includes/**');
    project.addFile('backends/' + name + '/sources/*unit.c*');  // 平台特定代碼
}
```

### 編譯前綴 (nocompile)

某些檔案只作為模板，不直接編譯：

```javascript
project.addFile('backends/' + name + '/sources/*', {nocompile: true});
project.addFile('backends/' + name + '/sources/*unit.c*');  // 只編譯 unit.c
```

### C 預處理器條件編譯

在 C 程式碼中使用條件編譯選擇正確的後端實作：

```c
#if defined(KORE_METAL)
    #include <kore3/metal/device_structs.h>
#elif defined(KORE_OPENGL)
    #include <kore3/opengl/device_structs.h>
#elif defined(KORE_VULKAN)
    #include <kore3/vulkan/device_structs.h>
#elif defined(KORE_DIRECT3D12)
    #include <kore3/direct3d12/device_structs.h>
#else
    #error("Unknown GPU backend")
#endif
```

## API 使用範例

### 新 GPU API 使用方式

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

### 程式碼風格

#### 命名慣例

| 元素 | 慣例 | 範例 |
|------|------|------|
| 函式 | snake_case | `kore_gpu_device_create` |
| 變數 | snake_case | `vertex_buffer` |
| 常數/列舉 | SCREAMING_SNAKE_CASE | `KORE_GPU_LOAD_OP_CLEAR` |
| 結構體 | snake_case (前綴) | `kore_gpu_device` |
| 類型定義 | snake_case + _t 後綴 | `kore_gpu_command_list_t` |
| 檔案 | snake_case | `device.h` |

#### 指標對齊

```c
// 正確：指標靠右對齊 (K&R 風格)
kore_gpu_device *device;

// 錯誤
kore_gpu_device* device;
```

#### 結構體初始化（指定初始化器）

```c
kore_gpu_color clear_color = {
    .r = 0.0f,
    .g = 0.0f,
    .b = 0.0f,
    .a = 1.0f,
};
```

#### 標頭檔結構

```c
#ifndef KORE_<MODULE>_HEADER
#define KORE_<MODULE>_HEADER

#include <kore3/global.h>
// ... 其他 include（按字母順序）

#ifdef __cplusplus
extern "C" {
#endif
// ... 宣告
#ifdef __cplusplus
}
#endif
#endif
```

#### Include 順序

1. Kore 公開標頭 (`<kore3/...>`)
2. 後端特定標頭 (`<kore3/metal/...>`)
3. Kong 標頭 (`<kong.h>`)
4. 系統 C 函式庫標頭 (`<assert.h>`, `<stdlib.h>`)

```c
#include <kore3/gpu/device.h>
#include <kore3/io/filereader.h>
#include <kong.h>
#include <assert.h>
```

#### 條件編譯

```c
#if defined(KORE_METAL)
#include <kore3/metal/device_structs.h>
#elif defined(KORE_OPENGL)
#include <kore3/opengl/device_structs.h>
#else
#error("Unknown GPU backend")
#endif
```

## API 狀態

| API | 狀態 |
|-----|------|
| **gpu** | ✅ 正常運作 - 使用 `includes/kore3/gpu/*` |
| graphics4 | ❌ 未實作 |
| graphics5 | ❌ 未實作 |

## GPU 架構分析

### 當前架構

```
┌─────────────────────────────────────────────────────────────┐
│                    公開 API (includes/kore3/gpu/)          │
│  device.h | buffer.h | commandlist.h | pipeline.h |       │
│  texture.h | sampler.h | fence.h | error.h | raytracing.h │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              統一實現層 (sources/gpu/*.c)                   │
│  使用 KORE_GPU_CALL 巨集根據編譯條件調用後端               │
│  device.c | buffer.c | commandlist.c | pipeline.c | ...  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              後端實現 (backends/gpu/<backend>/sources/)     │
│  metal/ | vulkan/ | opengl/ | direct3d11/ | webgpu/ | ... │
└─────────────────────────────────────────────────────────────┘
```

### 已實現的功能

| 模組 | 功能 | 狀態 |
|------|------|------|
| **Device** | 創建 GPU 設備、緩衝區、紋理、命令列表 | ✅ |
| **Buffer** | CPU/GPU 記憶體管理、鎖定/解鎖 | ✅ |
| **Texture** | 1D/2D/3D 紋理、視圖、深度紋理 | ✅ |
| **Pipeline** | 渲染管線、頂點/片段著色器 | ✅ |
| **CommandList** | 渲染通道、繪圖、拷貝命令 | ✅ |
| **Sampler** | 過濾器、尋址模式、各向異性 | ✅ |
| **Fence** | 同步機制 | ✅ |
| **Raytracing** | 光線追蹤 volume 和 hierarchy | ✅ |
| **Error** | 錯誤碼系統 | ✅ |
| **DescriptorSet** | 資源綁定（buffer、texture、sampler） | ✅ |

### 待完成的功能

| 功能 | 當前狀態 | 優先級 |
|------|----------|--------|
| **紋理貼圖完整實現** | 需要 Kongruent 擴展 + 後端實現 | 🔴 高 |
| **多渲染目標 (MRT)** | API 支援但可能不完整 | 🟡 中 |
| **Compute Shader** | 有基礎 API 但可能不完整 | 🟡 中 |
| **OpenGL 後端** | Descriptor Set 未實現 | 🟡 中 |
| **Direct3D 後端** | Descriptor Set 未實現 | 🟡 中 |

### 完善建議

#### 1. 紋理貼圖（高優先級）

需要實現：
- Kongruent 中添加紋理類型（如 `texture2D`）
- 添加采樣函數（如 `sample()`）
- 完善後端資源綁定實現
- 圖片加載輔助函式

#### 2. 跨平台驗證

測試 Vulkan、OpenGL 後端是否與 Metal 功能對等。

## 當前進度

### 已完成 (Phase 1-2)

#### GPU API 增強
- `includes/kore3/gpu/pipeline.h` - 統一管線類型
- `kore_gpu_buffer_unlock_all()` - 緩衝區解鎖函數
- `includes/kore3/gpu/error.h` - 錯誤處理系統（含錯誤碼）
- `kore_gpu_sampler` 支援 - 基礎 + mipmap + 各向異性
- `KORE_GPU_BUFFER_USAGE_VERTEX` 標誌

#### 紋理使用標誌
- 添加了 `KORE_GPU_TEXTURE_USAGE_SAMPLED = 0x0008`
- 添加了 `KORE_GPU_TEXTURE_USAGE_STORAGE = 0x0010`

#### Descriptor Set API ✅ 新增
- `includes/kore3/gpu/descriptorset.h` - 公開 API
- 類型：
  - `kore_gpu_descriptor_set_layout` - 描述符集佈局
  - `kore_gpu_descriptor_set` - 描述符集
  - `kore_gpu_descriptor_binding` - 資源綁定
- 函式：
  - `kore_gpu_descriptor_set_layout_create()` / `destroy()`
  - `kore_gpu_descriptor_set_create()` / `destroy()`
  - `kore_gpu_descriptor_set_set_buffer()` - 綁定緩衝區
  - `kore_gpu_descriptor_set_set_texture()` - 綁定紋理
  - `kore_gpu_descriptor_set_set_sampler()` - 綁定采樣器
- 後端實現：
  - Metal 後端：✅ 完成
  - Vulkan 後端：✅ 框架完成

#### 統一實現層
- `sources/gpu/descriptorset.c` - 使用 KORE_GPU_CALL 巨集

#### texture_test 示例
- 建立 `tests/texture_test/` 測試項目
- 基本四邊形渲染測試（UV 座標映射）

### 待完成 (Phase 3)

#### 紋理貼圖完整實現
- [ ] 在 Kongruent 中添加紋理類型支持（如 `texture2D`）
- [ ] 添加采樣函數（如 `sample()`）
- [ ] 完善 Metal 後端的資源綁定實現
- [ ] 添加圖片加載輔助函式（stb_image 集成）
- [ ] 創建完整紋理貼圖示例

#### 其他後端
- [ ] Vulkan 後端 descriptor set 完整實現
- [ ] OpenGL 後端實現
- [ ] Direct3D 11/12 後端實現

#### API 完善
- [ ] Compute Shader API 完善
- [ ] 多渲染目標 (MRT) 支持
- [ ] 紋理視圖 API

### GPU API 增強
- `includes/kore3/gpu/pipeline.h` - 統一管線類型
- `kore_gpu_buffer_unlock_all()` - 緩衝區解鎖函數
- `includes/kore3/gpu/error.h` - 錯誤處理系統（含錯誤碼）
- `kore_gpu_sampler` 支援 - 基礎 + mipmap + 各向異性
- `KORE_GPU_BUFFER_USAGE_VERTEX` 標誌

### Metal 後端
- Viewport 和 Scissor 設置
- 運行時著色器編譯（當無預設庫時）
- 深度緩衝區支持

### 紋理使用標誌
- 添加了 `KORE_GPU_TEXTURE_USAGE_SAMPLED` 和 `KORE_GPU_TEXTURE_USAGE_STORAGE`

### Descriptor Set API
- 新增 `includes/kore3/gpu/descriptorset.h` 公開 API
- 支援 buffer、texture、sampler 資源綁定
- Metal 後端實現完成
- Vulkan 後端框架完成

### texture_test 示例
- 建立 `tests/texture_test/` 測試項目
- 基本四邊形渲染測試

### 待完成：紋理貼圖
- 需要在 Kongruent 中添加紋理類型支持
- 需要完善 Metal 後端的資源綁定實現
- 需要圖片加載和紋理創建輔助函式

### Kongruent 著色器使用方式
cube_test 現在使用 Kongruent 著色器（Kore 的著色語言）而非硬編碼的 Metal 著色器：

**`tests/cube_test/shaders/shaders.kong`：**
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
    mvp: float4x4;
};

fun pos(input: vertex_in): vertex_out {
    var output: vertex_out;
    var position: float4;
    position.xyz = input.pos;
    position.w = 1.0;
    output.pos = constants.mvp * position;
    output.col = input.col;
    return output;
}

fun pix(input: vertex_out): float4 {
    var color: float4;
    color.xyz = input.col;
    color.w = 1.0;
    return color;
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

**Kongruent 關鍵慣例：**
- 頂點函式命名為 `pos`，片段函式命名為 `pix`
- 統一變數透過 `#[set(everything)]` 區塊
- 使用 `kong.h` 產生的 `constants_type` 作為統一緩衝區
- 使用 Kong 輔助函式：`kong_init()`、`kong_set_render_pipeline_pipeline()`、`kong_set_vertex_buffer_vertex_in()`、`kong_set_descriptor_set_everything()`、`constants_type_buffer_create()`

## cube_test 範例

完整範例參考 `tests/cube_test/sources/main.c`，包括：
- 從 `shaders/shaders.kong` 載入 Kongruent 著色器
- Kong 初始化 (`kong_init()`)
- 含位置+顏色屬性的頂點緩衝區創建
- 用於三角形索引的索引緩衝區
- 使用 `constants_type_buffer_create()` 的統一緩衝區
- 透過 `kong_create_everything_set()` 的描述符集合
- 透過 `kong_set_render_pipeline_pipeline()` 的渲染管線
- 含旋轉動畫的 MVP 矩陣計算
- 用於正確深度測試的深度紋理

**關鍵 Kong API 調用：**
```c
kong_init(&device);
constants_type_buffer_create(&device, &uniform_buffer, 1);
kong_create_everything_set(&device, &params, &uniform_set);

// 渲染迴圈中：
kong_set_render_pipeline_pipeline(&list);
kong_set_vertex_buffer_vertex_in(&list, &vertex_buffer);
kong_set_descriptor_set_everything(&list, &uniform_set);
```

## 關鍵檔案

| 檔案 | 說明 |
|------|------|
| `make` | 建置腳本（包裝 kmake） |
| `kfile.js` | 每個專案的建置配置 |
| `.clang-format` | 程式碼格式化規則 |
| `includes/kore3/` | 公開 API 標頭 |
| `sources/` | 實作 |
| `backends/gpu/` | GPU 後端實作 |

## 常見任務

### 建立新測試

```bash
mkdir tests/<testname>/sources
# 1. 建立 tests/<testname>/kfile.js
# 2. 將源檔案加入 sources/
# 3. 建置：./make -g metal --kore . --from tests/<testname> --compile
```

### 新增 API 函式

1. 在 `includes/kore3/` 的標頭中新增宣告
2. 在 `sources/` 中新增實作
3. 在 `backends/gpu/<backend>/sources/` 中新增後端特定實作
4. 格式化：`clang-format -style=file -i <files>`

## 參考資源

- [Kore 官方網站](https://github.com/Kode/Kore)
- [Kongruent 著色語言](https://github.com/Kode/Kongruent)
- [Kore Samples](https://github.com/Kode/Kore-Samples)
