# axmol — Engine Port

**Source:** https://github.com/axmolengine/axmol (tag/branch for v2.3.1)
**Runtime:** clib4 (`-mcrt=clib4`)

## What

Port the axmol 2.3.1 engine to AmigaOS4 with:
- Vulkan-only rendering (`AX_RENDER_API=vk`) via VulkanOS4
- AmigaOS4 platform layer (Application, FileUtils, Device)
- application.library v2 integration (game mode, prefs, notifications)
- PROGDIR: paths, tooltypes, Workbench startup, icon, version string

## Platform files to create

- `PlatformConfig.h` — add AX_PLATFORM_AMIGAOS4
- `Application-amigaos4.cpp` — app lifecycle + application.library
- `FileUtils-amigaos4.cpp` — PROGDIR: paths, tooltypes
- `Device-amigaos4.cpp` — DPI, device info
- `Common-amigaos4.cpp` — Intuition EasyRequest()
- CMake toolchain file for AmigaOS4 cross-compilation

## Build

```
# From Docker clib4-dev:os4-gcc11
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/amigaos4.cmake -DAX_RENDER_API=vk ..
make
```
