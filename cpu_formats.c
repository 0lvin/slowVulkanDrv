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
#include <stdbool.h>
#include <vulkan/vk_icd.h>

void cpu_GetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties)
{
	VkFormatFeatureFlags linear = 0, tiled = 0, buffer = 0;

	printf("Loader asked about %d format\n", format);

	// TODO: return really known formats

	pFormatProperties->linearTilingFeatures = linear;
	pFormatProperties->optimalTilingFeatures = tiled;
	pFormatProperties->bufferFeatures = buffer;
}

VkResult cpu_GetPhysicalDeviceImageFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	VkImageTiling                               tiling,
	VkImageUsageFlags                           usage,
	VkImageCreateFlags                          createFlags,
	VkImageFormatProperties*                    pImageFormatProperties)
{
	printf("format: %d type: %d tiling: %d usage: %d  createFlags: %d\n",
		format, type, tiling, usage, createFlags);

	// TODO: return real support types

	*pImageFormatProperties = (VkImageFormatProperties) {
		.maxExtent = { 0, 0, 0 },
		.maxMipLevels = 0,
		.maxArrayLayers = 0,
		.sampleCounts = 0,
		.maxResourceSize = 0,
	};

	return VK_ERROR_FORMAT_NOT_SUPPORTED;
}

void cpu_GetPhysicalDeviceSparseImageFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	uint32_t                                    samples,
	VkImageUsageFlags                           usage,
	VkImageTiling                               tiling,
	uint32_t*                                   pNumProperties,
	VkSparseImageFormatProperties*              pProperties)
{
	/* Sparse images are not yet supported. */
	*pNumProperties = 0;
}
