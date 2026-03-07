# backends/system - System Backend Implementations

**Parent:** `./AGENTS.md` | **287 files, 13 subdirectories**

## OVERVIEW
Platform-specific system code (windowing, threads, file I/O, audio).

## STRUCTURE
```
backends/system/
├── macos/       # macOS native
├── ios/         # iOS native
├── apple/      # Shared Apple code
├── windows/    # Windows native
├── microsoft/  # Shared Windows code
├── linux/      # Linux native
├── posix/      # POSIX shared
├── android/    # Android native
├── wasm/       # WebAssembly
├── emscripten/ # Emscripten
├── freebsd/    # FreeBSD
├── windowsapp/ # UWP
└── kompjuta/   # Compute-only
```

## WHERE TO LOOK
| Platform | Primary Use |
|----------|-------------|
| macos/ | Desktop macOS |
| ios/ | iOS/tvOS |
| windows/ | Desktop Windows |
| linux/ | Desktop Linux |
| android/ | Android |
| wasm/ | Browser WASM |

## CONVENTIONS (THIS DIR)
- Each platform dir has: window, thread, audio, display implementations
- Use KORE_<PLATFORM> defines for conditional compilation

## ANTI-PATTERNS
- NEVER put platform-agnostic code here
- NEVER assume cross-platform — always use conditional compilation
