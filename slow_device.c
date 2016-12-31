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
		printf("Loader asked about '%s' function, but we dont have such(%s).\n", pName, __func__);
	}
	return result;
}

PFN_vkVoidFunction slow_GetDeviceProcAddr(
	VkDevice                                    device,
	const char*                                 pName)
{
	void * result = NULL;
	if (!pName) {
		printf("Loader asked something strange!\n");
		return NULL;
	}
	printf("Loader asked about '%s' function.\n", pName);
	result = slow_lookup_entrypoint(pName);
	if (!result) {
		printf("Loader asked about '%s' function, but we dont have such(%s).\n", pName, __func__);
	}
	return result;
}

VkResult slow_EnumeratePhysicalDevices(
	VkInstance                                  _instance,
	uint32_t*                                   pPhysicalDeviceCount,
	VkPhysicalDevice*                           pPhysicalDevices)
{
	// TODO: enumerate devices

	*pPhysicalDeviceCount = 0;

	return VK_SUCCESS;
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
	*pInstance = malloc(sizeof(VkInstance));

	// TODO: implement create

	return VK_SUCCESS;
}

void slow_DestroyInstance(
	VkInstance                                  instance,
	const VkAllocationCallbacks*                pAllocator)
{
	// TODO: implement cleanup
	free(instance);
}

VkResult slow_EnumerateInstanceExtensionProperties(
	const char*                                 pLayerName,
	uint32_t*                                   pPropertyCount,
	VkExtensionProperties*                      pProperties)
{
	if (pProperties == NULL) {
		*pPropertyCount = ARRAY_SIZE(global_extensions);
		return VK_SUCCESS;
	}

	*pPropertyCount = MIN2(*pPropertyCount, ARRAY_SIZE(global_extensions));
	typed_memcpy(pProperties, global_extensions, *pPropertyCount);

	if (*pPropertyCount < ARRAY_SIZE(global_extensions))
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

void slow_GetPhysicalDeviceFeatures(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceFeatures*                   pFeatures)
{

	memset(pFeatures, 0, sizeof(*pFeatures));

	*pFeatures = (VkPhysicalDeviceFeatures) {
		.robustBufferAccess                       = false,
		.fullDrawIndexUint32                      = false,
		.imageCubeArray                           = false,
		.independentBlend                         = false,
		.geometryShader                           = false,
		.tessellationShader                       = false,
		.sampleRateShading                        = false,
		.dualSrcBlend                             = false,
		.logicOp                                  = false,
		.multiDrawIndirect                        = false,
		.drawIndirectFirstInstance                = false,
		.depthClamp                               = false,
		.depthBiasClamp                           = false,
		.fillModeNonSolid                         = false,
		.depthBounds                              = false,
		.wideLines                                = false,
		.largePoints                              = false,
		.alphaToOne                               = false,
		.multiViewport                            = false,
		.samplerAnisotropy                        = false,
		.textureCompressionETC2                   = false,
		.textureCompressionASTC_LDR               = false,
		.textureCompressionBC                     = false,
		.occlusionQueryPrecise                    = false,
		.pipelineStatisticsQuery                  = false,
		.vertexPipelineStoresAndAtomics           = false,
		.fragmentStoresAndAtomics                 = false,
		.shaderTessellationAndGeometryPointSize   = false,
		.shaderImageGatherExtended                = false,
		.shaderStorageImageExtendedFormats        = false,
		.shaderStorageImageMultisample            = false,
		.shaderUniformBufferArrayDynamicIndexing  = false,
		.shaderSampledImageArrayDynamicIndexing   = false,
		.shaderStorageBufferArrayDynamicIndexing  = false,
		.shaderStorageImageArrayDynamicIndexing   = false,
		.shaderStorageImageReadWithoutFormat      = false,
		.shaderStorageImageWriteWithoutFormat     = false,
		.shaderClipDistance                       = false,
		.shaderCullDistance                       = false,
		.shaderFloat64                            = false,
		.shaderInt64                              = false,
		.shaderInt16                              = false,
		.alphaToOne                               = false,
		.variableMultisampleRate                  = false,
		.inheritedQueries                         = false,
	};
}


VkResult slow_CreateDevice(
	VkPhysicalDevice                            physicalDevice,
	const VkDeviceCreateInfo*                   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDevice*                                   pDevice)
{
	// TODO: create device

	return VK_ERROR_OUT_OF_HOST_MEMORY;
}

void slow_GetPhysicalDeviceProperties(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceProperties*                 pProperties)
{
	VkSampleCountFlags sample_counts = 0xf;
	VkPhysicalDeviceLimits limits = {
		.maxImageDimension1D                      = (1 << 14),
		.maxImageDimension2D                      = (1 << 14),
		.maxImageDimension3D                      = (1 << 11),
		.maxImageDimensionCube                    = (1 << 14),
		.maxImageArrayLayers                      = (1 << 11),
		.maxTexelBufferElements                   = 128 * 1024 * 1024,
		.maxUniformBufferRange                    = UINT32_MAX,
		.maxStorageBufferRange                    = UINT32_MAX,
		.maxPushConstantsSize                     = MAX_PUSH_CONSTANTS_SIZE,
		.maxMemoryAllocationCount                 = UINT32_MAX,
		.maxSamplerAllocationCount                = 64 * 1024,
		.bufferImageGranularity                   = 64, /* A cache line */
		.sparseAddressSpaceSize                   = 0,
		.maxBoundDescriptorSets                   = MAX_SETS,
		.maxPerStageDescriptorSamplers            = 64,
		.maxPerStageDescriptorUniformBuffers      = 64,
		.maxPerStageDescriptorStorageBuffers      = 64,
		.maxPerStageDescriptorSampledImages       = 64,
		.maxPerStageDescriptorStorageImages       = 64,
		.maxPerStageDescriptorInputAttachments    = 64,
		.maxPerStageResources                     = 128,
		.maxDescriptorSetSamplers                 = 256,
		.maxDescriptorSetUniformBuffers           = 256,
		.maxDescriptorSetUniformBuffersDynamic    = 256,
		.maxDescriptorSetStorageBuffers           = 256,
		.maxDescriptorSetStorageBuffersDynamic    = 256,
		.maxDescriptorSetSampledImages            = 256,
		.maxDescriptorSetStorageImages            = 256,
		.maxDescriptorSetInputAttachments         = 256,
		.maxVertexInputAttributes                 = 32,
		.maxVertexInputBindings                   = 32,
		.maxVertexInputAttributeOffset            = 2047,
		.maxVertexInputBindingStride              = 2048,
		.maxVertexOutputComponents                = 128,
		.maxTessellationGenerationLevel           = 0,
		.maxTessellationPatchSize                 = 0,
		.maxTessellationControlPerVertexInputComponents = 0,
		.maxTessellationControlPerVertexOutputComponents = 0,
		.maxTessellationControlPerPatchOutputComponents = 0,
		.maxTessellationControlTotalOutputComponents = 0,
		.maxTessellationEvaluationInputComponents = 0,
		.maxTessellationEvaluationOutputComponents = 0,
		.maxGeometryShaderInvocations             = 32,
		.maxGeometryInputComponents               = 64,
		.maxGeometryOutputComponents              = 128,
		.maxGeometryOutputVertices                = 256,
		.maxGeometryTotalOutputComponents         = 1024,
		.maxFragmentInputComponents               = 128,
		.maxFragmentOutputAttachments             = 8,
		.maxFragmentDualSrcAttachments            = 2,
		.maxFragmentCombinedOutputResources       = 8,
		.maxComputeSharedMemorySize               = 32768,
		.maxComputeWorkGroupCount                 = { 65535, 65535, 65535 },
		.maxComputeWorkGroupInvocations           = 16 * 1024,
		.maxComputeWorkGroupSize = {
			16 * 1024/*devinfo->max_cs_threads*/,
			16 * 1024,
			16 * 1024
		},
		.subPixelPrecisionBits                    = 4 /* FIXME */,
		.subTexelPrecisionBits                    = 4 /* FIXME */,
		.mipmapPrecisionBits                      = 4 /* FIXME */,
		.maxDrawIndexedIndexValue                 = UINT32_MAX,
		.maxDrawIndirectCount                     = UINT32_MAX,
		.maxSamplerLodBias                        = 16,
		.maxSamplerAnisotropy                     = 16,
		.maxViewports                             = MAX_VIEWPORTS,
		.maxViewportDimensions                    = { (1 << 14), (1 << 14) },
		.viewportBoundsRange                      = { INT16_MIN, INT16_MAX },
		.viewportSubPixelBits                     = 13, /* We take a float? */
		.minMemoryMapAlignment                    = 4096, /* A page */
		.minTexelBufferOffsetAlignment            = 1,
		.minUniformBufferOffsetAlignment          = 4,
		.minStorageBufferOffsetAlignment          = 4,
		.minTexelOffset                           = -8,
		.maxTexelOffset                           = 7,
		.minTexelGatherOffset                     = -8,
		.maxTexelGatherOffset                     = 7,
		.minInterpolationOffset                   = 0, /* FIXME */
		.maxInterpolationOffset                   = 0, /* FIXME */
		.subPixelInterpolationOffsetBits          = 0, /* FIXME */
		.maxFramebufferWidth                      = (1 << 14),
		.maxFramebufferHeight                     = (1 << 14),
		.maxFramebufferLayers                     = (1 << 10),
		.framebufferColorSampleCounts             = sample_counts,
		.framebufferDepthSampleCounts             = sample_counts,
		.framebufferStencilSampleCounts           = sample_counts,
		.framebufferNoAttachmentsSampleCounts     = sample_counts,
		.maxColorAttachments                      = MAX_RTS,
		.sampledImageColorSampleCounts            = sample_counts,
		.sampledImageIntegerSampleCounts          = VK_SAMPLE_COUNT_1_BIT,
		.sampledImageDepthSampleCounts            = sample_counts,
		.sampledImageStencilSampleCounts          = sample_counts,
		.storageImageSampleCounts                 = VK_SAMPLE_COUNT_1_BIT,
		.maxSampleMaskWords                       = 1,
		.timestampComputeAndGraphics              = false,
		.timestampPeriod                          = 10.0, /* FINISHME */
		.maxClipDistances                         = 8,
		.maxCullDistances                         = 8,
		.maxCombinedClipAndCullDistances          = 8,
		.discreteQueuePriorities                  = 1,
		.pointSizeRange                           = { 0.125, 255.875 },
		.lineWidthRange                           = { 0.0, 7.9921875 },
		.pointSizeGranularity                     = (1.0 / 8.0),
		.lineWidthGranularity                     = (1.0 / 128.0),
		.strictLines                              = false, /* FINISHME */
		.standardSampleLocations                  = true,
		.optimalBufferCopyOffsetAlignment         = 128,
		.optimalBufferCopyRowPitchAlignment       = 128,
		.nonCoherentAtomSize                      = 64,
	};

	*pProperties = (VkPhysicalDeviceProperties) {
		.apiVersion = VK_MAKE_VERSION(1, 0, 5),
		.driverVersion = 1,
		.vendorID = 0,
		.deviceID = 0,
		.deviceType = VK_PHYSICAL_DEVICE_TYPE_CPU,
		.limits = limits,
		.sparseProperties = {0}, /* Broadwell doesn't do sparse. */
	};

	strcpy(pProperties->deviceName, "slow_fake");

	// TODO: create device
}

void slow_GetPhysicalDeviceMemoryProperties(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceMemoryProperties*           pMemoryProperties)
{
	// TODO: return real memory types
	pMemoryProperties->memoryTypeCount = 0;

	pMemoryProperties->memoryHeapCount = 0;
}

void slow_GetPhysicalDeviceQueueFamilyProperties(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pCount,
	VkQueueFamilyProperties*                    pQueueFamilyProperties)
{
	if (pQueueFamilyProperties == NULL) {
		*pCount = 1;
		return;
	}
	assert(*pCount >= 1);

	*pQueueFamilyProperties = (VkQueueFamilyProperties) {
		.queueFlags = VK_QUEUE_GRAPHICS_BIT |
		VK_QUEUE_COMPUTE_BIT |
		VK_QUEUE_TRANSFER_BIT,
		.queueCount = 1,
		.timestampValidBits = 64,
		.minImageTransferGranularity = (VkExtent3D) { 1, 1, 1 },
	};
}

VkResult slow_EnumerateDeviceExtensionProperties(
	VkPhysicalDevice                            physicalDevice,
	const char*                                 pLayerName,
	uint32_t*                                   pPropertyCount,
	VkExtensionProperties*                      pProperties)
{
	if (pProperties == NULL) {
		*pPropertyCount = ARRAY_SIZE(device_extensions);
		return VK_SUCCESS;
	}

	*pPropertyCount = MIN2(*pPropertyCount, ARRAY_SIZE(device_extensions));
	typed_memcpy(pProperties, device_extensions, *pPropertyCount);

	if (*pPropertyCount < ARRAY_SIZE(device_extensions))
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}
