# AmigaOS4 OGLES2 Implementation Plan

## Goal

Replace GLAD with direct ogles2.library access for axmol v2.3.1 on AmigaOS4,
using the GLES 2.0 code path (`AX_GLES_PROFILE=200`).

## Rendering Stack

```
Games → axmol (GLES 2.0) → ogles2.library → Warp3D Nova → RadeonHD/RX GPU
```

## Key Technical Facts

### How GLAD works (current)
- `glad/gl.h` declares global function pointers: `PFNGLVIEWPORTPROC glad_glViewport = NULL;`
- Defines macros: `#define glViewport glad_glViewport`
- At runtime, `gladLoadGLES2(loader)` populates all pointers via a callback
- axmol code calls `glViewport(...)` → expands to `glad_glViewport(...)` → function pointer call

### How ogles2.library works (target)
- `ogles2_defs.h` provides GL types + constants
- `interfaces/ogles2.h` defines `struct OGLES2IFace` with all GL function pointers
- `inline4/ogles2.h` defines macros: `#define glViewport(x,y,w,h) IOGLES2->glViewport((x),(y),(w),(h))`
- `proto/ogles2.h` declares `extern struct OGLES2IFace *IOGLES2`
- `libogles2.a` autoinit opens ogles2.library and populates IOGLES2 before main()
- `libGLESv2.a` provides standalone C function symbols (wrapper stubs around IOGLES2->)

### Why GLAD and ogles2 inline4 macros CANNOT coexist
Both define `#define glViewport ...` with incompatible expansions. The ogles2 interface
functions also take an extra `Self` parameter that GLAD's PFN types don't include.

### The AmigaOS4 gl2platform.h problem
The SDK's `GLES2/gl2platform.h` defines `GL_ES_VERSION_2_0` before `gl2.h` processes,
which causes `gl2.h` to skip ALL standard function declarations. Functions are expected
to come from either `inline4/ogles2.h` macros or `libGLESv2.a` symbols.

### GCC PCH limitation
GCC precompiled headers do NOT preserve `#define` macros from `inline4/ogles2.h`.
This means the `__USE_INLINE__` approach fails when PCH is enabled. Without PCH,
many axmol files have missing includes (they rely on PCH for implicit includes).

### exec/types.h conflicts
- `#define STATIC static` — conflicts with `BufferUsage::STATIC` enum value
- `#define FLOAT float32` / `#define DOUBLE float64` — guarded by `NO_OLD_TYPE_DEFINES`

---

## Strategy: AmigaOS4-Specific GLAD Replacement

Create a replacement `glad/gl.h` for AmigaOS4 that provides GL function declarations
compatible with `libGLESv2.a` linking. Do NOT use `__USE_INLINE__` or `inline4/ogles2.h`.
Do NOT use the AmigaOS4-modified `GLES2/gl2platform.h`.

### Why libGLESv2.a (not inline4 macros)
- `libGLESv2.a` provides real C function symbols (`T glViewport` in nm output)
- These are standard C function calls — no `Self` parameter, no interface dispatch
- Works with GCC PCH (real declarations, not macros)
- No exec/types.h pollution (we control which headers are included)

### Why our own headers (not SDK GLES2 headers)
- SDK's `gl2platform.h` hijacks the include chain with AmigaOS-specific routing
- It defines `GL_ES_VERSION_2_0` which blocks `gl2.h`'s function declarations
- It pulls in `exec/types.h` → `STATIC`, `FLOAT`, `DOUBLE` macro conflicts

---

## What Gets Removed

| Item | Why |
|------|-----|
| `vulkan-sdk/` directory | Not needed for GLES2 rendering |
| GLAD library compilation (`glad/src/gl.c`) | Replaced by libGLESv2.a |
| `libGL.a` link dependency | Desktop GL not used |

## What Gets Added

### New files (6 files)

1. **`axmol/shim/glad/gl.h`** — Replaces GLAD for AmigaOS4
   - Includes our clean GLES2 headers (below)
   - Defines GLAD compat stubs (`gladLoadGL`, `gladLoadGLES2` → return 1)
   - Defines `GLAD_GL 1` for code that checks it

2. **`axmol/shim/GLES2/gl2.h`** — Clean Khronos GLES2 header
   - Copy from SDK but with our clean `gl2platform.h`
   - Contains ALL GL type definitions and function declarations
   - NOT guarded by `GL_ES_VERSION_2_0` (our platform header doesn't define it)

3. **`axmol/shim/GLES2/gl2platform.h`** — Minimal platform defines
   ```c
   #ifndef __gl2platform_h_
   #define __gl2platform_h_
   #include <KHR/khrplatform.h>
   #ifndef GL_APICALL
   #define GL_APICALL
   #endif
   #ifndef GL_APIENTRY
   #define GL_APIENTRY
   #endif
   #endif
   ```
   NO AmigaOS routing. NO `#define GL_ES_VERSION_2_0`. NO `#include <proto/ogles2.h>`.

4. **`axmol/shim/GLES2/gl2ext.h`** — Copy from SDK (standard Khronos extensions)

5. **`axmol/shim/KHR/khrplatform.h`** — Copy from SDK (standard Khronos types)

6. **`core/platform/amigaos4/GL-amigaos4.h`** — AmigaOS4 GL routing header
   ```c
   #pragma once
   #include "glad/gl.h"

   /* Map desktop GL names to GLES2 OES equivalents (same pattern as GL-android.h) */
   #undef GL_DEPTH_STENCIL
   #undef GL_DEPTH24_STENCIL8
   #undef GL_UNSIGNED_INT_24_8
   #define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
   #define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
   #define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES
   #define glClearDepth glClearDepthf
   #define glMapBuffer glMapBufferOES
   #define glUnmapBuffer glUnmapBufferOES
   #define glBindVertexArray glBindVertexArrayOES
   #define glDeleteVertexArrays glDeleteVertexArraysOES
   #define glGenVertexArrays glGenVertexArraysOES
   ```
   Modeled on `GL-android.h` — maps desktop GL names to GLES2 equivalents.

### Modified files in axmol submodule (fork)

7. **`core/platform/PlatformConfig.h`** — Add AmigaOS4 GLES2 profile
   ```c
   #ifndef AX_GLES_PROFILE
   #    if defined(__ANDROID__)
   #        define AX_GLES_PROFILE 200
   #    elif (AX_TARGET_PLATFORM == AX_PLATFORM_WINRT)
   #        define AX_GLES_PROFILE 300
   #    elif (AX_TARGET_PLATFORM == AX_PLATFORM_AMIGAOS4)
   #        define AX_GLES_PROFILE 200        /* ← NEW */
   #    else
   #        define AX_GLES_PROFILE 0
   #    endif
   #endif
   ```

8. **`core/platform/GL.h`** — Route AmigaOS4 to new GL header
   ```c
   #elif AX_TARGET_PLATFORM == AX_PLATFORM_AMIGAOS4
   #    include "platform/amigaos4/GL-amigaos4.h"    /* was: linux/GL-linux.h */
   ```

9. **`core/axmol.h`** — Update AmigaOS4 section
   ```c
   #if (AX_TARGET_PLATFORM == AX_PLATFORM_AMIGAOS4)
   #    include "platform/amigaos4/Application-amigaos4.h"
   #    include "platform/GLViewImpl.h"
   #    include "platform/amigaos4/GL-amigaos4.h"    /* was: linux/GL-linux.h */
   #    include "platform/amigaos4/StdC-amigaos4.h"
   #endif
   ```

10. **`core/renderer/backend/Enums.h`** — Rename STATIC to AX_STATIC
    (Plus all 27 usage sites — patch already created)

11. **`core/renderer/backend/opengl/OpenGLState.h`** — Guard GL3-only constants
    ```c
    #if AX_GLES_PROFILE >= 300 || !AX_GLES_PROFILE
        GL_UNIFORM_BUFFER,
        GL_PIXEL_PACK_BUFFER,
    #else
        0,  // not in GLES2
        0,  // not in GLES2
    #endif
    ```

12. **`core/platform/GLViewImpl.cpp`** — Guard FBO extension loading for GLES2
    ```c
    #if AX_GLES_PROFILE
    static bool loadFboExtensions() { return true; }  // FBO is core in GLES2
    #else
    static bool loadFboExtensions() { /* existing code */ }
    #endif
    ```

### Modified files in standalone build

13. **`axmol/CMakeLists.txt`** — Build system changes
    - Remove GLAD library build
    - Remove `glad` from link libraries
    - Add `AX_GLES_PROFILE=200` to compile definitions
    - Add `NO_OLD_TYPE_DEFINES` to compile definitions
    - Add `axmol/shim/` to include path FIRST (before 3rdparty)
    - Link: `GLESv2` + `ogles2` (instead of `GL`)

14. **`lords-of-midnight/CMakeLists.txt`** — Same compile definition + link changes

15. **`Makefile`** — Remove vulkan-sdk target

---

## Include Chain (How It All Connects)

### PCH compilation of axmol.h
```
axmol.h
  → PlatformConfig.h
    → AX_GLES_PROFILE = 200 (for AmigaOS4)
    → AX_USE_GL = 1 (non-Apple)
  → platform/amigaos4/GL-amigaos4.h
    → glad/gl.h (OUR SHIM at axmol/shim/glad/gl.h)
      → GLES2/gl2.h (OUR CLEAN COPY at axmol/shim/GLES2/gl2.h)
        → GLES2/gl2platform.h (OUR CLEAN COPY — no AmigaOS routing)
          → KHR/khrplatform.h (standard types)
        → GL type definitions (GLenum, GLfloat, etc)
        → GL constant definitions (#define GL_TRIANGLES, etc)
        → GL function declarations (GL_APICALL void GL_APIENTRY glViewport(...))
      → GLES2/gl2ext.h (standard extensions)
      → GLAD stubs (gladLoadGLES2 returns 1)
    → GLES2 OES mappings (GL_DEPTH_STENCIL → GL_DEPTH_STENCIL_OES, etc)
```

### At link time
```
axmol objects + game objects
  → libGLESv2.a (provides glViewport, glBindFramebuffer, etc as real symbols)
  → libogles2.a (autoinit: opens ogles2.library, gets IOGLES2 before main)
  → libglfw3.a (windowing, ogles2 context via aglCreateContext2)
```

### At runtime
```
libogles2.a autoinit → opens ogles2.library → gets IOGLES2 interface
libGLESv2.a functions → internally call through IOGLES2->
GLFW → aglCreateContext2 → creates ogles2 rendering context on Intuition window
axmol → calls glViewport() etc → resolved to libGLESv2.a → ogles2.library → Warp3D Nova
```

---

## Why This Works

1. **No macro conflicts**: Our shim headers don't pull in exec/types.h or AmigaOS routing
2. **PCH compatible**: Function declarations are real C declarations, not macros
3. **Standard linking**: libGLESv2.a provides standard function symbols
4. **ogles2.library still used**: libGLESv2.a internally routes through IOGLES2
5. **libogles2.a autoinit**: Opens the library before main — IOGLES2 is ready
6. **GLES2 code path active**: AX_GLES_PROFILE=200 enables all existing GLES2 conditionals
7. **Android-proven**: The GLES2 code path is already tested on Android

## Risks

| Risk | Mitigation |
|------|------------|
| ogles2_defs.h type differences (GLsizeiptr = int vs khronos_ssize_t) | Both 32-bit on PPC — verify with test |
| Missing GL extensions in shim headers | Copy gl2ext.h from SDK which has extensions |
| GLFW needs ogles2_defs.h for CreateContextTags | GLFW has its own include paths — uses SDK headers directly |
| libGLESv2.a may not have all extension functions | Check nm output; add ogles2 extensions as needed |

## Verification Steps

1. shim/GLES2/gl2.h compiles standalone with standard function declarations
2. No exec/types.h macros leak into axmol compilation
3. PCH compiles successfully with GL function declarations preserved
4. All 238 axmol core source files compile
5. Game links successfully against libGLESv2.a + libogles2.a
6. Both LOM and DDR build and produce PPC ELF binaries
