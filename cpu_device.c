#include <stdio.h>
#include "cpu_private.h"
#include "vk_alloc.h"
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
	// printf("Loader asked about '%s' function.\n", pName);
	result = cpu_lookup_entrypoint(pName);
	if (!result) {
		printf("Loader asked about '%s' function, but we dont have such(%s).\n", pName, __func__);
	}
	return result;
}

PFN_vkVoidFunction cpu_GetDeviceProcAddr(
	VkDevice                                    device,
	const char*                                 pName)
{
	void * result = NULL;
	if (!pName) {
		printf("Loader asked something strange!\n");
		return NULL;
	}
	// printf("Loader asked about '%s' function.\n", pName);
	result = cpu_lookup_entrypoint(pName);
	if (!result) {
		printf("Loader asked about '%s' function, but we dont have such(%s).\n", pName, __func__);
	}
	return result;
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

VkResult cpu_CreateInstance(
	const VkInstanceCreateInfo*                 pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkInstance*                                 pInstance)
{
	struct cpu_instance *instance;

	assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);

	uint32_t client_version;
	if (pCreateInfo->pApplicationInfo &&
	    pCreateInfo->pApplicationInfo->apiVersion != 0) {
		client_version = pCreateInfo->pApplicationInfo->apiVersion;
	} else {
		client_version = VK_MAKE_VERSION(1, 0, 0);
	}

	if (VK_MAKE_VERSION(1, 0, 0) > client_version ||
	    client_version > VK_MAKE_VERSION(1, 0, 0xfff)) {
		printf("Client requested version %d.%d.%d\n",
				 VK_VERSION_MAJOR(client_version),
				 VK_VERSION_MINOR(client_version),
				 VK_VERSION_PATCH(client_version));
		return VK_ERROR_INCOMPATIBLE_DRIVER;
	}

	for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
		bool found = false;
		for (uint32_t j = 0; j < ARRAY_SIZE(global_extensions); j++) {
			if (strcmp(pCreateInfo->ppEnabledExtensionNames[i],
				   global_extensions[j].extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found)
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	instance = vk_alloc2(&default_alloc, pAllocator, sizeof(*instance), 8,
			       VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
	if (!instance)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	instance->_loader_data.loaderMagic = ICD_LOADER_MAGIC;

	if (pAllocator)
		instance->alloc = *pAllocator;
	else
		instance->alloc = default_alloc;

	instance->apiVersion = client_version;
	instance->physicalDeviceCount = -1;

	*pInstance = cpu_instance_to_handle(instance);

	return VK_SUCCESS;
}

VkResult cpu_EnumeratePhysicalDevices(
	VkInstance                                  _instance,
	uint32_t*                                   pPhysicalDeviceCount,
	VkPhysicalDevice*                           pPhysicalDevices)
{
	CPU_FROM_HANDLE(cpu_instance, instance, _instance);

	if (instance->physicalDeviceCount < 0) {
		struct cpu_physical_device* pDevice;
		pDevice = vk_alloc2(&default_alloc, &instance->alloc, sizeof(*pDevice), 8,
			       VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
		if (!pDevice)
			return VK_ERROR_OUT_OF_HOST_MEMORY;

		pDevice->_loader_data.loaderMagic = ICD_LOADER_MAGIC;

		pDevice->instance = instance;

		instance->physicalDeviceCount = 1;

		instance->physicalDevices[0] = cpu_physical_device_to_handle(pDevice);
	}

	if (pPhysicalDevices && (instance->physicalDeviceCount > 0)) {
		for(int i = 0; i < instance->physicalDeviceCount; i++) {
			pPhysicalDevices[i] = instance->physicalDevices[i];
		}
	}
	*pPhysicalDeviceCount = instance->physicalDeviceCount;

	return VK_SUCCESS;
}

void cpu_DestroyInstance(
	VkInstance                                  _instance,
	const VkAllocationCallbacks*                pAllocator)
{
	CPU_FROM_HANDLE(cpu_instance, instance, _instance);

	if (instance->physicalDeviceCount > 0) {
		for(int i = 0; i < instance->physicalDeviceCount; i++) {
			vk_free(&instance->alloc, instance->physicalDevices[i]);
		}
	}

	vk_free(&instance->alloc, instance);
}

VkResult cpu_EnumerateInstanceExtensionProperties(
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

void cpu_GetPhysicalDeviceFeatures(
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


VkResult cpu_CreateDevice(
	VkPhysicalDevice                            physicalDevice,
	const VkDeviceCreateInfo*                   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDevice*                                   pDevice)
{
	CPU_FROM_HANDLE(cpu_physical_device, physical_device, physicalDevice);
	struct cpu_device *device;

	for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
		bool found = false;
		for (uint32_t j = 0; j < ARRAY_SIZE(device_extensions); j++) {
			if (strcmp(pCreateInfo->ppEnabledExtensionNames[i],
				   device_extensions[j].extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found)
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	device = vk_alloc2(&physical_device->instance->alloc, pAllocator,
			     sizeof(*device), 8,
			     VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
	if (!device)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	device->_loader_data.loaderMagic = ICD_LOADER_MAGIC;
	device->instance = physical_device->instance;

	if (pAllocator)
		device->alloc = *pAllocator;
	else
		device->alloc = physical_device->instance->alloc;

	*pDevice = cpu_device_to_handle(device);
	return VK_SUCCESS;
}

void cpu_DestroyDevice(
	VkDevice                                    _device,
	const VkAllocationCallbacks*                pAllocator)
{
	CPU_FROM_HANDLE(cpu_device, device, _device);
	vk_free(&device->alloc, device);
}

void cpu_GetPhysicalDeviceProperties(
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
		.sparseProperties = {0},
	};

	strcpy(pProperties->deviceName, "cpu_fake");

	// TODO: create device
}

void cpu_GetPhysicalDeviceMemoryProperties(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceMemoryProperties*           pMemoryProperties)
{
	// TODO: return real memory types
	pMemoryProperties->memoryTypeCount = 1;
	pMemoryProperties->memoryTypes[0] = (VkMemoryType) {
		.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.heapIndex = 1, // index in memoryHeaps
	};

	pMemoryProperties->memoryHeapCount = 1;
	pMemoryProperties->memoryHeaps[0] = (VkMemoryHeap) {
		.size = 256 * 1024 * 1024,
		.flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
	};
}

VkResult cpu_AllocateMemory(
	VkDevice                                    _device,
	const VkMemoryAllocateInfo*                 pAllocateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDeviceMemory*                             pMem)
{
	CPU_FROM_HANDLE(cpu_device, device, _device);
	struct cpu_device_memory *mem;
	VkResult result;
	assert(pAllocateInfo->sType == VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	if (pAllocateInfo->allocationSize == 0) {
		/* Apparently, this is allowed */
		*pMem = VK_NULL_HANDLE;
		return VK_SUCCESS;
	}

	mem = vk_alloc2(&device->alloc, pAllocator, sizeof(*mem), 8,
			  VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if (mem == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	mem->map = vk_alloc2(&device->alloc, pAllocator, pAllocateInfo->allocationSize, 8,
			  VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	mem->map_size = pAllocateInfo->allocationSize;
	if (!mem->map) {
		result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
		goto fail;
	}
	mem->type_index = pAllocateInfo->memoryTypeIndex;

	*pMem = cpu_device_memory_to_handle(mem);

	return VK_SUCCESS;

fail:
	vk_free2(&device->alloc, pAllocator, mem);

	return result;
}

VkResult cpu_MapMemory(
	VkDevice                                    _device,
	VkDeviceMemory                              _memory,
	VkDeviceSize                                offset,
	VkDeviceSize                                size,
	VkMemoryMapFlags                            flags,
	void**                                      ppData)
{
	CPU_FROM_HANDLE(cpu_device, device, _device);
	CPU_FROM_HANDLE(cpu_device_memory, mem, _memory);

	if (mem == NULL) {
		*ppData = NULL;
		return VK_SUCCESS;
	}

	printf("Map device: %p mem: %p\n", device, mem);

	*ppData = mem->map;
	if (*ppData) {
		*ppData += offset;
		return VK_SUCCESS;
	}

	// TODO implement map

	return VK_ERROR_MEMORY_MAP_FAILED;
}

void cpu_UnmapMemory(
	VkDevice                                    _device,
	VkDeviceMemory                              _memory)
{
	CPU_FROM_HANDLE(cpu_device, device, _device);
	CPU_FROM_HANDLE(cpu_device_memory, mem, _memory);

	if (mem == NULL)
		return;

	// TODO implement unmap
	printf("Unmap device: %p mem: %p\n", device, mem);
}

VkResult cpu_CreateBuffer(
	VkDevice                                    _device,
	const VkBufferCreateInfo*                   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBuffer*                                   pBuffer)
{
	// TODO implement create buffer
	return VK_ERROR_OUT_OF_HOST_MEMORY;
}

void cpu_GetPhysicalDeviceQueueFamilyProperties(
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

VkResult cpu_EnumerateDeviceExtensionProperties(
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
