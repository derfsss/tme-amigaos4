# vulkan-sdk — VulkanOS4 SDK for AmigaOS4

**Source:** https://github.com/derfsss/VulkanOS4

Headers and clib4-compatible link library (`libvulkan_loader.a`) from the
VulkanOS4 project. Used by GLFW for Vulkan surface creation.

## Contents

- `include/` — 9 SDK headers (vulkan core, VK_AMIGA_surface extension, AmigaOS interfaces)
- `src/vulkan_autoinit.c` — Auto-init stub (opens vulkan.library before main)
- `test/test_vulkan_init.c` — Verification test program
- `Makefile` — Builds `libvulkan_loader.a` with clib4

## Build

```
make          # builds lib/libvulkan_loader.a
make verify   # checks exported symbols
make test     # builds test binary (runs on AmigaOS4 only)
make clean
```

## Notes

- `libvulkan_loader.a` is a clib4 autoinit stub — it opens `vulkan.library`
  via GCC constructor before `main()` and closes it via destructor after
- `vulkan.library` itself is NOT rebuilt — it manages its own newlib internally
- VK_AMIGA_surface creates Vulkan surfaces from Intuition Window* handles
