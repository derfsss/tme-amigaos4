/*
 * test_vulkan_init.c — Verify VulkanOS4 SDK integration with clib4
 *
 * Tests:
 * 1. vulkan.library auto-opened by constructor (VulkanBase != NULL)
 * 2. VulkanIFace obtained (IVulkan != NULL)
 * 3. vkEnumerateInstanceVersion returns valid Vulkan version
 * 4. VK_AMIGA_surface extension is advertised
 * 5. Loader version is valid
 *
 * Build: make test
 * Run:   build/test_vulkan_init  (on AmigaOS4 only)
 */

#include <proto/exec.h>
#include <proto/vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_amiga.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* These are provided by libvulkan_loader.a autoinit */
extern struct Library     *VulkanBase;
extern struct VulkanIFace *IVulkan;

static int test_count   = 0;
static int pass_count   = 0;
static int fail_count   = 0;

static void test(const char *name, int condition)
{
    test_count++;
    if (condition)
    {
        printf("  PASS: %s\n", name);
        pass_count++;
    }
    else
    {
        printf("  FAIL: %s\n", name);
        fail_count++;
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("vulkan-sdk: VulkanOS4 integration test (clib4)\n");
    printf("=============================================\n\n");

    /* Test 1: Library auto-opened */
    test("VulkanBase != NULL (vulkan.library opened)", VulkanBase != NULL);
    if (VulkanBase == NULL)
    {
        printf("\n  FATAL: vulkan.library not found.\n");
        printf("  Ensure VulkanOS4 is installed:\n");
        printf("    - LIBS:vulkan.library must exist\n");
        printf("    - DEVS:Vulkan/icd.d/ must contain at least one .json manifest\n");
        printf("\nResults: %d/%d passed, %d failed\n",
               pass_count, test_count, fail_count);
        return 1;
    }

    /* Test 2: Interface obtained */
    test("IVulkan != NULL (VulkanIFace obtained)", IVulkan != NULL);
    if (IVulkan == NULL)
    {
        printf("\n  FATAL: VulkanIFace not available.\n");
        printf("\nResults: %d/%d passed, %d failed\n",
               pass_count, test_count, fail_count);
        return 1;
    }

    /* Test 3: Query Vulkan version */
    uint32_t version = 0;
    VkResult result = vkEnumerateInstanceVersion(&version);
    test("vkEnumerateInstanceVersion succeeds (VK_SUCCESS)",
         result == VK_SUCCESS);
    test("Vulkan version >= 1.1.0",
         VK_API_VERSION_MAJOR(version) >= 1 &&
         VK_API_VERSION_MINOR(version) >= 1);

    printf("\n  Vulkan version: %d.%d.%d\n",
           VK_API_VERSION_MAJOR(version),
           VK_API_VERSION_MINOR(version),
           VK_API_VERSION_PATCH(version));

    /* Test 4: Check for VK_AMIGA_surface extension */
    uint32_t ext_count = 0;
    result = vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL);
    test("vkEnumerateInstanceExtensionProperties succeeds", result == VK_SUCCESS);

    int found_amiga_surface = 0;
    int found_khr_surface   = 0;

    if (result == VK_SUCCESS && ext_count > 0)
    {
        VkExtensionProperties *exts = (VkExtensionProperties *)
            malloc(ext_count * sizeof(VkExtensionProperties));
        if (exts != NULL)
        {
            result = vkEnumerateInstanceExtensionProperties(
                NULL, &ext_count, exts);

            printf("\n  Instance extensions (%u):\n", (unsigned)ext_count);
            for (uint32_t i = 0; i < ext_count; i++)
            {
                printf("    - %s (v%u)\n",
                       exts[i].extensionName, exts[i].specVersion);

                if (strcmp(exts[i].extensionName,
                           VK_AMIGA_SURFACE_EXTENSION_NAME) == 0)
                    found_amiga_surface = 1;
                if (strcmp(exts[i].extensionName,
                           VK_KHR_SURFACE_EXTENSION_NAME) == 0)
                    found_khr_surface = 1;
            }
            free(exts);
        }
    }

    test("VK_KHR_surface extension present", found_khr_surface);
    test("VK_AMIGA_surface extension present", found_amiga_surface);

    /* Test 5: Loader version */
    uint32_t loader_ver = VkAmigaGetLoaderVersion();
    printf("\n  Loader version: %d.%d\n",
           VK_API_VERSION_MAJOR(loader_ver),
           VK_API_VERSION_MINOR(loader_ver));
    test("Loader version > 0", loader_ver > 0);

    /* Summary */
    printf("\n=============================================\n");
    printf("Results: %d/%d passed, %d failed\n",
           pass_count, test_count, fail_count);

    return (fail_count > 0) ? 1 : 0;
}
