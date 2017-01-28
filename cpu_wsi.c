#include <stdio.h>
#include "cpu_private.h"
#include "vk_alloc.h"
#include <stdbool.h>

VkResult cpu_GetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                _surface,
    VkBool32*                                   pSupported)
{
	// TODO: return real support
	*pSupported = false;
	return VK_SUCCESS;
}

VkResult cpu_GetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                _surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
	// TODO: return real support
	return VK_INCOMPLETE;
}

VkResult cpu_GetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                _surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats)
{
	// TODO: return real support
	return VK_INCOMPLETE;
}

VkResult cpu_GetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                _surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes)
{
	// TODO: return real support
	return VK_INCOMPLETE;
}

void cpu_DestroySurfaceKHR(
    VkInstance                                   _instance,
    VkSurfaceKHR                                 _surface,
    const VkAllocationCallbacks*                 pAllocator)
{
	// TODO: return real support
}
