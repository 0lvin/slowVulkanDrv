/*
 * Copyright © 2017 Denis Pauk
 *
 * based in part on anv and radv drivers which is:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Bas Nieuwenhuizen
 * Copyright © 2016 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include "cpu_private.h"
#include "vk_alloc.h"

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

VkResult cpu_CreateSwapchainKHR(
	VkDevice                                     _device,
	const VkSwapchainCreateInfoKHR*              pCreateInfo,
	const VkAllocationCallbacks*                 pAllocator,
	VkSwapchainKHR*                              pSwapchain)
{
	// TODO: return real support
	return VK_INCOMPLETE;
}
