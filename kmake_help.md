# kmake 详细使用指南

kmake 是 Kore 的元构建系统，基于 Node.js，用于生成跨平台项目文件并编译。

## 基本语法

```bash
./make [选项] --from <项目路径> --kore <Kore目录>
```

## 命令行选项

### 项目配置

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `--from <path>` | 项目位置 | `.` |
| `--to <path>` | 构建输出目录 | `build` |
| `-k, --kore <dir>` | Kore 目录位置 | - |
| `--kfile <file>` | 项目配置文件名 | `kfile.js` |
| `--name <name>` | 项目名称 (init时) | `Project` |

### 目标平台

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `-t, --target <name>` | 目标平台 | `osx` |
| `-g, --graphics <name>` | 图形 API | `default` |
| `-a, --audio <name>` | 音频 API | `default` |
| `--arch <name>` | 目标架构 | `default` |
| `--vr <name>` | VR 设备 | `none` |

### 编译选项

| 选项 | 说明 |
|------|------|
| `--compile` | 编译可执行文件 |
| `--run` | 运行可执行文件 |
| `--debug` | 调试模式编译 |
| `--lib` | 编译为静态库 |
| `--dynlib` | 编译为动态库 |
| `--pch` | 使用预编译头 (C++) |
| `--cores <n>` | 使用 CPU 核心数 |
| `-v, --visualstudio <ver>` | VS 版本 (vs2022) |

### 着色器

| 选项 | 说明 |
|------|------|
| `--noshaders` | 不编译着色器 |
| `--onlyshaders` | 只编译着色器 |
| `--outputintermediatespirv` | 输出中间 SPIRV (调试) |

### Web/Emscripten

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `--server` | 启动 HTTP 服务器 | - |
| `--port <n>` | 服务器端口 | `8080` |

### iOS/macOS

| 选项 | 说明 |
|------|------|
| `--nosigning` | 禁用代码签名 |

### 工具链

| 选项 | 说明 |
|------|------|
| `--compiler <path>` | 指定编译器 |
| `--cc <path>` | C 编译器路径 |
| `--cxx <path>` | C++ 编译器路径 |
| `--ar <path>` | ar 工具路径 |
| `--strip <path>` | strip 工具路径 |

### 其他

| 选项 | 说明 |
|------|------|
| `--init` | 初始化新项目 |
| `--open` | 打开创建的项目 |
| `--vscode` | 创建 VSCode 项目 |
| `--meson` | 导出 meson 构建文件 |
| `--json` | 导出项目 JSON 描述 |
| `--stdout` | 输出到 stdout |
| `--update` | 更新 Kore 及子模块 |
| `--option <k,k=v>` | 传递选项到 kfile |
| `--nosymlinks` | 不跟随符号链接 |
| `--dev <path>` | 使用指定 kmake 源码 |

## 目标平台

| 平台 | 说明 | 默认图形 API |
|------|------|--------------|
| `osx` | macOS | Metal |
| `ios` | iOS | Metal |
| `tvos` | Apple TV | Metal |
| `android` | Android | Vulkan |
| `windows` | Windows 桌面 | Direct3D12 |
| `windowsapp` | Windows UWP | Direct3D11 |
| `linux` | Linux | Vulkan |
| `freebsd` | FreeBSD | OpenGL |
| `emscripten` | Web (Emscripten) | OpenGL (WebGL) |
| `wasm` | WebAssembly | OpenGL |
| `ps4` / `ps5` | PlayStation (需插件) | - |
| `xboxone` / `xboxseries` | Xbox (需插件) | - |
| `switch` / `switch2` | Nintendo Switch (需插件) | - |

## 图形 API

| API | 支持平台 |
|-----|----------|
| `metal` | macOS, iOS, tvOS |
| `opengl` | 全平台 |
| `vulkan` | Windows, Linux, Android |
| `direct3d11` | Windows |
| `direct3d12` | Windows |
| `webgpu` | Emscripten, Wasm |
| `default` | 使用平台默认 |

## VR API

| VR | 平台 |
|----|------|
| `oculus` | Windows |
| `steamvr` | Windows |
| `hololens` | Windows UWP |
| `none` | 无 VR |

## 常用命令示例

### macOS

```bash
# Metal 构建
./make -g metal --kore . --from tests/cube_test --compile
open build/build/Release/Cube-Test.app

# OpenGL 构建
./make -g opengl --kore . --from tests/empty --compile

# 调试构建
./make -g metal --kore . --from tests/cube_test --debug --compile
```

### iOS

```bash
# 模拟器 (无签名)
./make -g metal --kore . --from tests/shader-gpu -t ios --compile --nosigning
# 在 Xcode 中打开并运行
open build/Shader-GPU.xcodeproj
```

### Android

```bash
./make -g vulkan --kore . --from tests/empty -t android --compile
```

### Web

```bash
# WebGPU
./make -g webgpu --kore . --from tests/empty -t emscripten --compile --server
# 打开 http://localhost:8080

# WebGL
./make -g opengl --kore . --from tests/empty -t emscripten --compile --server
```

### 项目管理

```bash
# 只生成项目文件 (不编译)
./make -g metal --kore . --from tests/cube_test

# 初始化新项目
mkdir my_game && cd my_game
../make --init --name MyGame --kore ..

# 创建 VSCode 项目
./make -g metal --kore . --from my_game --vscode

# 导出 meson 构建文件
./make -g metal --kore . --from tests/empty --meson
```

## kfile.js 配置

### 基本结构

```javascript
const project = new Project('MyProject');

// 添加 Kore 依赖
await project.addProject(findKore());

// 添加源文件
project.addFile('sources/**');

// 添加 Kongruent 着色器目录
project.addKongDir('shaders');

// 设置资源目录
project.setDebugDir('deployment');

// 扁平化并解析
project.flatten();
resolve(project);
```

### 常用方法

| 方法 | 说明 |
|------|------|
| `addFile(pattern)` | 添加源文件 (glob 模式) |
| `addIncludeDir(path)` | 添加头文件目录 |
| `addDefine(name)` | 添加预处理器定义 |
| `addLib(name)` | 添加链接库 |
| `addKongDir(path)` | 添加 Kongruent 着色器目录 |
| `setDebugDir(path)` | 设置调试资源目录 |
| `addProject(path)` | 添加子项目 |
| `flatten()` | 扁平化项目结构 |

### flatten() 详解

`project.flatten()` 的作用是**扁平化源文件目录结构**。

#### 不使用 flatten()
```
项目结构:                    生成的 IDE 文件:
sources/                     sources/foo.c
  foo.c                      sources/bar/baz.c
  bar/
    baz.c
```

#### 使用 flatten()
```
项目结构:                    生成的 IDE 文件:
sources/                     foo.c
  foo.c                      baz.c    (路径被扁平化)
  bar/
    baz.c
```

#### 主要目的

1. **简化项目结构** - 所有源文件在 IDE 中显示在同一层级
2. **减少路径深度** - 避免深层嵌套的目录结构
3. **统一编译** - 所有文件使用相同的编译选项

#### 使用建议

```javascript
// 简单项目 - 使用 flatten
project.addFile('sources/**');
project.flatten();
resolve(project);

// 复杂项目 - 不使用 flatten (保持目录结构)
project.addFile('sources/**');
resolve(project);
```

Kore 测试项目都使用 `flatten()`，而 Kore 主项目不使用，以保持复杂的后端目录结构。

### 使用自定义选项

在 kfile.js 中:
```javascript
if (Options.myFeature) {
    project.addDefine('ENABLE_FEATURE');
}
```

命令行传递:
```bash
./make --option myFeature --kore . --from my_project
```

传递键值对:
```bash
./make --option myFeature=true,debugLevel=2 --kore . --from my_project
```

## 构建流程

```
kfile.js
    ↓
kmake 解析配置
    ↓
调用 kongruent 编译 .kong 着色器
    ↓
生成平台项目文件
    ↓
调用编译器
    ↓
输出可执行文件
```

## 工具位置

| 工具 | 位置 |
|------|------|
| kmake | `tools/macos_arm64/kmake` |
| kongruent | `tools/macos_arm64/kongruent` |

工具通过 `./get_dlc` 脚本下载 (Git 子模块)。
