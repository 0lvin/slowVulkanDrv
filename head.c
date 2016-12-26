#include <stdio.h>
#include "slow_private.h"
#include "vk_alloc.h"
#include <stdbool.h>
#include <vulkan/vk_icd.h>

VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion){
	if (!pSupportedVersion) {
		printf("Loader asked something strange!\n");
		return VK_ERROR_INCOMPATIBLE_DRIVER;
	}
	if (*pSupportedVersion < 2) {
		printf("Loader asked about outdated version %d.%d.%d\n"
			"vkGetInstanceProcAddr is deprecated!",
			VK_VERSION_MAJOR(*pSupportedVersion),
			VK_VERSION_MINOR(*pSupportedVersion),
			VK_VERSION_PATCH(*pSupportedVersion));

		return VK_ERROR_INCOMPATIBLE_DRIVER;
	}

	printf("Loader asked about '%d' version.\n", *pSupportedVersion);

	/* codebase tested only with second loader version */
	*pSupportedVersion = 2;

	return VK_SUCCESS;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName){
	void * result = NULL;
	if (!pName) {
		printf("Loader asked something strange!\n");
		return NULL;
	}
	printf("Loader asked about '%s' function.\n", pName);
	result = slow_lookup_entrypoint(pName);
	if (!result) {
		printf("Loader asked about '%s' function, but we dont have such.\n", pName);
	}
	return slow_lookup_entrypoint(pName);
}

static const VkExtensionProperties global_extensions[] = {
	{
		.extensionName = VK_KHR_SURFACE_EXTENSION_NAME,
		.specVersion = 25,
	},
#ifdef VK_USE_PLATFORM_XCB_KHR
	{
		.extensionName = VK_KHR_XCB_SURFACE_EXTENSION_NAME,
		.specVersion = 6,
	},
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
	{
		.extensionName = VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
		.specVersion = 6,
	},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	{
		.extensionName = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
		.specVersion = 5,
	},
#endif
};

static const VkExtensionProperties device_extensions[] = {
	{
		.extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		.specVersion = 68,
	},
};

static void *
default_alloc_func(void *pUserData, size_t size, size_t align,
                   VkSystemAllocationScope allocationScope)
{
	return malloc(size);
}

static void *
default_realloc_func(void *pUserData, void *pOriginal, size_t size,
                     size_t align, VkSystemAllocationScope allocationScope)
{
	return realloc(pOriginal, size);
}

static void
default_free_func(void *pUserData, void *pMemory)
{
	free(pMemory);
}

static const VkAllocationCallbacks default_alloc = {
	.pUserData = NULL,
	.pfnAllocation = default_alloc_func,
	.pfnReallocation = default_realloc_func,
	.pfnFree = default_free_func,
};

VkResult slow_CreateInstance(
	const VkInstanceCreateInfo*                 pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkInstance*                                 pInstance)
{
	return VK_ERROR_INCOMPATIBLE_DRIVER;
}

void slow_DestroyInstance(
	VkInstance                                  _instance,
	const VkAllocationCallbacks*                pAllocator)
{
}
