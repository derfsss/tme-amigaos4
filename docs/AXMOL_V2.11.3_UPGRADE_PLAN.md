# Axmol v2.3.1 → v2.11.3 Upgrade Plan

## Overview

Upgrade the axmol submodule from v2.3.1 to v2.11.3 (latest v2.x LTS).
Our AmigaOS4+OGLES2 patches (50 files, 880 lines) need to be reapplied
to v2.11.3 which has 450 upstream commits of changes.

## Patch Testing Results

`git apply --check` against v2.11.3 shows:
- **42 files apply cleanly** (new platform files, AX_STATIC rename, clib4/PPC fixes, GLES2 guards)
- **7 files fail** (context changes in surrounding code)
- **1 file missing** (GLViewImpl.cpp renamed to RenderViewImpl.cpp)

## Approach

Create a new branch `amigaos4-v2.11.3` from the v2.11.3 tag, then apply
patches in order. Use `git apply --3way` for fuzzy matching, then manually
resolve the 8 conflicts.

## Patch Categories

### Category A: Apply Cleanly (42 files)
No manual work needed. These include:
- All 7 new amigaos4/ platform files (Application, FileUtils, Device, Common, StdC, GL-amigaos4.h)
- All 19 AX_STATIC rename files
- clib4/PPC fixes (endian_portable.hpp, string_view.hpp, mio.hpp, Logging.cpp, FileStream.cpp, Image.cpp)
- .gitignore

### Category B: Context Changes — Need Manual Resolution (7 files)

#### B1. `1k/platforms.cmake`
**Issue:** Indentation changed from 4 spaces to 2 spaces
**Fix:** Add AmigaOS4 elseif with 2-space indentation before `else()`:
```cmake
elseif(${CMAKE_SYSTEM_NAME} MATCHES "AmigaOS")
  set(AMIGAOS4 TRUE)
  set(PLATFORM_NAME amigaos4)
```

#### B2. `cmake/Modules/AXBuildHelpers.cmake`
**Issue:** Indentation changed
**Fix:** Same logic, 2-space indentation:
```cmake
if(NOT PWSH_PROG AND NOT AMIGAOS4)
  message("powershell not found.")
  message(FATAL_ERROR "...")
endif()
```

#### B3. `cmake/Modules/AXSLCC.cmake`
**Issue:** Indentation changed
**Fix:** Same logic, 2-space indentation:
```cmake
if(NOT AXSLCC_EXE)
  if(AMIGAOS4)
    message(STATUS "axslcc not found — skipping for AmigaOS4 cross-build")
  else()
    message(STATUS, "axslcc not found.")
    message(FATAL_ERROR "...")
  endif()
endif()
```

#### B4. `core/axmol.h`
**Issue:** GLViewImpl → RenderViewImpl rename in all platform blocks
**Fix:** Insert AmigaOS4 block after WASM block using v2.11.3 naming:
```cpp
#if (AX_TARGET_PLATFORM == AX_PLATFORM_AMIGAOS4)
#    include "platform/amigaos4/Application-amigaos4.h"
#    include "platform/RenderViewImpl.h"
#    include "platform/amigaos4/GL-amigaos4.h"
#    include "platform/amigaos4/StdC-amigaos4.h"
#endif
```

#### B5. `core/base/Controller.h`
**Issue:** Header guard changed from `#ifndef` to `#pragma once`
**Fix:** Add `AX_TARGET_PLATFORM == AX_PLATFORM_AMIGAOS4 ||` to the platform guard
(same change, just different line number due to `#pragma once`)

#### B6. `core/platform/CMakeLists.txt`
**Issue:** Indentation changed, RenderViewImpl rename, added Device-desktop.cpp
**Fix:** Add AmigaOS4 elseif with 2-space indentation, use RenderViewImpl naming:
```cmake
elseif(AMIGAOS4)
  set(_AX_PLATFORM_SPECIFIC_HEADER
    platform/amigaos4/Application-amigaos4.h
    platform/amigaos4/FileUtils-amigaos4.h
    platform/amigaos4/StdC-amigaos4.h
    platform/RenderViewImpl.h
  )
  set(_AX_PLATFORM_SPECIFIC_SRC
    platform/amigaos4/Application-amigaos4.cpp
    platform/amigaos4/FileUtils-amigaos4.cpp
    platform/amigaos4/Common-amigaos4.cpp
    platform/amigaos4/Device-amigaos4.cpp
    platform/RenderViewImpl.cpp
  )
```
Also add AMIGAOS4 to the Device-desktop.cpp platform guard:
```cmake
if((WIN32 AND NOT WINRT) OR LINUX OR MACOSX OR WASM OR AMIGAOS4)
  list(APPEND _AX_PLATFORM_SPECIFIC_SRC "platform/desktop/Device-desktop.cpp")
endif()
```

#### B7. `core/platform/StdC.h`
**Issue:** Header guard changed from `#ifndef` to `#pragma once`
**Fix:** Same AmigaOS4 include routing, just different line position

### Category C: File Renamed (1 file)

#### C1. `core/platform/GLViewImpl.cpp` → `core/platform/RenderViewImpl.cpp`
**Changes in our patch:**
1. FBO extension loading guarded for GLES2 — apply to RenderViewImpl.cpp
2. GLAD loading skipped for AmigaOS4 — apply to RenderViewImpl.cpp

The function signatures and logic are the same, just the file/class name changed.

### Category D: v2.11.3 API Changes (game-side patches)

These affect the game code, not axmol itself:

#### D1. `messageBox` → `showAlert`
`Common-amigaos4.cpp` must implement `showAlert(string_view, string_view, AlertStyle)`
instead of `messageBox(const char*, const char*)`

#### D2. Mouse event callbacks (lords-of-midnight only)
`uipanel.cpp` lambda: `[](Event*)` → `[](EventMouse*) -> bool`

#### D3. TMXTiledMap API
`buildWithMapInfo(mapInfo)` → `buildWithMapInfo(mapInfo, true)` (bool arg restored in v2.11.3)

### Category E: v2.11.3-Specific New Requirements

#### E1. `axmolver.h` — Generated version header
Must create from `axmolver.h.in` template with v2.11.3 values.
Not in the git diff (it's a generated file). Create manually.

#### E2. Ogg/Opus exclusion in standalone CMakeLists.txt
v2.11.3 adds opus source files to the ogg directory.
Our standalone CMakeLists.txt ogg glob filter needs: `list(FILTER OGG_SRC EXCLUDE REGEX "opus/")`

#### E3. `fmtlib` — Must compile from bundled source
v2.11.3 uses fmt 11.1.1 (bundled). SDK's libfmt has ABI incompatibilities.
Already handled in our standalone CMakeLists.txt.

## Implementation Steps

### Step 1: Generate patches from current v2.3.1 branch
```bash
cd axmol/src
git format-patch v2.3.1..HEAD --stdout > /tmp/amigaos4-patches.mbox
```

### Step 2: Create new branch from v2.11.3
```bash
git fetch upstream --tags
git checkout -b amigaos4-v2.11.3 v2.11.3
```

### Step 3: Apply clean patches with 3-way merge
```bash
git apply --3way /tmp/amigaos4-ogles2-full.patch
```
This will apply 42 files cleanly and mark 8 as conflicted.

### Step 4: Resolve 7 context conflicts (B1-B7)
Manual edits with correct indentation and naming.

### Step 5: Handle renamed file (C1)
Apply FBO guard and GLAD skip patches to RenderViewImpl.cpp manually.

### Step 6: Handle v2.11.3 API changes (D1-D3)
Update Common-amigaos4.cpp, verify game patches.

### Step 7: Handle new requirements (E1-E3)
Generate axmolver.h, update standalone CMakeLists.txt.

### Step 8: Test build
```bash
make clean && make all
```

### Step 9: Update game patches if needed
Rebuild lords-of-midnight/CMakeLists.txt for any v2.11.3 API changes.

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| New GLES2-incompatible code in v2.11.3 | Medium | Test iteratively; same GLES2 path as Android |
| New 3rdparty deps in standalone build | Low | Add to CMakeLists.txt as needed |
| Renderer changes break OGLES2 | Low | Core renderer backend is stable between v2.x releases |
| Game API breakage | Low | Only 3 known changes (D1-D3), all documented |

## Estimated Changes

- **Automatic (git apply):** 42 files
- **Manual conflict resolution:** 8 files
- **API adaptation:** 3 files
- **New/generated:** 2 files (axmolver.h, opus exclusion)
- **Total manual work:** ~13 files need hand-editing
