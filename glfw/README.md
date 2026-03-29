# glfw — GLFW 3.5 with VulkanOS4 Surface Support

**Source:** https://github.com/afxgroup/glfw (fork with GLFW_PLATFORM_OS4)
**Runtime:** clib4 (`-mcrt=clib4`)

## What

Patch Andrea Palmate's GLFW 3.5 AmigaOS4 port to add Vulkan surface creation
via VulkanOS4's VK_AMIGA_surface extension. The existing port has full Intuition
windowing but stubs out the 3 Vulkan functions.

## Patch required

In `src/os4_context.c`, implement:
1. `_glfwGetRequiredInstanceExtensionsOS4()` → return "VK_AMIGA_surface"
2. `_glfwGetPhysicalDevicePresentationSupportOS4()` → return GLFW_TRUE
3. `_glfwCreateWindowSurfaceOS4()` → call vkCreateAmigaSurfaceAMIGA() with
   Intuition Window* from GLFW's OS4 platform state

## Build

```
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/amigaos4.cmake -DGLFW_BUILD_OS4=ON -DBUILD_SHARED_LIBS=OFF ..
make
```
