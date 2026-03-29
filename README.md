# axmol_ports — AmigaOS4 Port of axmol Engine

Porting the axmol engine to AmigaOS4 to run The Lords of Midnight.

## Rendering Stack

```
Lords of Midnight → axmol (OpenGL) → GLFW (GL context) → GL4ES → OGLES2/Warp3D Nova → GPU
```

> **Note:** The original plan assumed axmol v2.3.1 had a Vulkan backend. It does not — only
> OpenGL and Metal are available. Rendering goes through GL4ES which translates OpenGL to
> OGLES2/Warp3D Nova for the GPU hardware.

## Sub-projects

| Directory | Source | Purpose |
|-----------|--------|---------|
| `axmol/` | https://github.com/axmolengine/axmol (v2.3.1) | Game engine — OpenGL backend, platform layer for AmigaOS4 |
| `glfw/` | https://github.com/afxgroup/glfw | GLFW 3.5 with OS4 backend — patch Vulkan stubs for VK_AMIGA_surface |
| `openal-soft/` | axmol's bundled 3rdparty/openal + AHI backend | OpenAL Soft 1.25.1 — add AHI audio backend for AmigaOS4 |
| `vulkan-sdk/` | VulkanOS4 SDK headers/libs | VulkanOS4 headers, link library, VK_AMIGA_surface extension |
| `lords-of-midnight/` | https://github.com/ChilliHugger/The-Lords-Of-Midnight | The game — build against ported axmol |

## Build Requirements

- **C runtime:** clib4 exclusively (`-mcrt=clib4`) — no newlib or clib2 mixing
- **Compiler:** ppc-amigaos-gcc 11.5 via Docker `clib4-dev:os4-gcc11` (derived from `walkero/amigagccondocker:os4-gcc11-sdk53.34` with latest clib4)
- **SDK:** AmigaOS4 SDK 54.16 at `../../docs/sdk/`
- **clib4 libs:** `../../docs/sdk/local/clib4/lib/` (freetype, zlib, libpng, libjpeg, fmt, etc.)

## Reference Documents

- `PLAN.md` — Master implementation plan with build order and dependencies
- `AMIGAOS4_INTEGRATION.md` — OS4 integration code templates and risk matrix
- Each sub-project has its own `CLAUDE.md`, `README.md`, and `PLAN.md`
