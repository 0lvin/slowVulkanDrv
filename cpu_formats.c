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
