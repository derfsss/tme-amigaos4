/*
 * vulkan_autoinit.c — Auto-initialisation stub for VulkanOS4
 *
 * Links into application as a static library. GCC constructor/destructor
 * attributes ensure vulkan.library is opened before main() and closed
 * after main() returns.
 *
 * CRT-agnostic: uses only AmigaOS Exec API. Compatible with clib4, newlib,
 * and clib2.
 *
 * Build: ppc-amigaos-gcc -mcrt=clib4 -O2 -Wall -Werror
 *        -I ../include -c vulkan_autoinit.c -o vulkan_autoinit.o
 */

#include <exec/exec.h>
#include <interfaces/exec.h>
#include <proto/exec.h>

#include <interfaces/vulkan.h>

/*
 * Global symbols — applications and inline4 macros reference these directly.
 * Initialised to NULL; set by constructor, cleared by destructor.
 */
struct Library      *VulkanBase = NULL;
struct VulkanIFace  *IVulkan    = NULL;

/*
 * Minimum library version required. VulkanOS4 v1.0 = version 1.
 */
#define VULKAN_LIB_MIN_VERSION 1

/*
 * Constructor: opens vulkan.library and obtains the main interface.
 *
 * Called automatically by the C runtime before main(). If the library
 * cannot be opened (e.g., not installed), VulkanBase and IVulkan remain
 * NULL. Applications must check for this before calling Vulkan functions.
 *
 * We deliberately do NOT call exit() or abort() on failure — the
 * application should handle missing Vulkan gracefully (e.g., display
 * an error requester).
 */
void __init_vulkan(void) __attribute__((constructor, used));
void __init_vulkan(void)
{
    VulkanBase = IExec->OpenLibrary("vulkan.library", VULKAN_LIB_MIN_VERSION);
    if (VulkanBase == NULL)
    {
        IExec->DebugPrintF("[vulkan_autoinit] "
            "Failed to open vulkan.library version %ld\n",
            (long)VULKAN_LIB_MIN_VERSION);
        return;
    }

    IVulkan = (struct VulkanIFace *)IExec->GetInterface(
        VulkanBase, "main", 1, NULL);
    if (IVulkan == NULL)
    {
        IExec->DebugPrintF("[vulkan_autoinit] "
            "Failed to get VulkanIFace from vulkan.library\n");
        IExec->CloseLibrary(VulkanBase);
        VulkanBase = NULL;
        return;
    }
}

/*
 * Destructor: releases the interface and closes the library.
 *
 * Called automatically by the C runtime after main() returns (or when
 * exit() is called). Safe to call even if the constructor failed —
 * checks for NULL before each operation.
 */
void __exit_vulkan(void) __attribute__((destructor, used));
void __exit_vulkan(void)
{
    if (IVulkan != NULL)
    {
        IExec->DropInterface((struct Interface *)IVulkan);
        IVulkan = NULL;
    }

    if (VulkanBase != NULL)
    {
        IExec->CloseLibrary(VulkanBase);
        VulkanBase = NULL;
    }
}
