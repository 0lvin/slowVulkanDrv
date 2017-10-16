#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan.h>

#ifdef VK_USE_PLATFORM_XCB_KHR
#include <X11/Xlib-xcb.h> /* for XGetXCBConnection() */
#endif

VkInstance init_instance()
{
	VkResult err;
	uint32_t i;
	int found_surface_extensions = 0;
	uint32_t instance_extension_count;

	VkInstance vulkan_instance;

	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	if (err == VK_SUCCESS || instance_extension_count > 0)
	{
		VkExtensionProperties *instance_extensions = (VkExtensionProperties *)
						malloc(sizeof(VkExtensionProperties) * instance_extension_count);
		err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);

		for (i = 0; i < instance_extension_count; ++i)
		{
			if (strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
			{
				found_surface_extensions++;
			}

			if (strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
			{
				found_surface_extensions++;
			}
		}

		free(instance_extensions);
	}

	if(found_surface_extensions != 2) {
		SDL_Log("Couldn't find %s/%s extensions", VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME);
		return NULL;
	}

	VkApplicationInfo application_info;
	memset(&application_info, 0, sizeof(application_info));
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName = "triangle";
	application_info.applicationVersion = 1;
	application_info.pEngineName = "triangle";
	application_info.engineVersion = 1;
	application_info.apiVersion = VK_API_VERSION_1_0;

	const char * const instance_extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME };

	VkInstanceCreateInfo instance_create_info;
	memset(&instance_create_info, 0, sizeof(instance_create_info));
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &application_info;
	instance_create_info.enabledExtensionCount = 2;
	instance_create_info.ppEnabledExtensionNames = instance_extensions;

	err = vkCreateInstance(&instance_create_info, NULL, &vulkan_instance);
	if (err != VK_SUCCESS) {
		SDL_Log("Couldn't create Vulkan instance");
		return NULL;
	}
	return vulkan_instance;
}

VkSurfaceKHR init_surface(VkInstance vulkan_instance, SDL_SysWMinfo *sys_wm_info)
{
	VkResult err;
	uint32_t i;
	VkSurfaceKHR vulkan_surface;

	VkXcbSurfaceCreateInfoKHR surface_create_info;
	memset(&surface_create_info, 0, sizeof(surface_create_info));
	surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surface_create_info.connection = XGetXCBConnection((Display*) sys_wm_info->info.x11.display);
	surface_create_info.window = sys_wm_info->info.x11.window;

	err = vkCreateXcbSurfaceKHR(vulkan_instance, &surface_create_info, NULL, &vulkan_surface);
	if (err != VK_SUCCESS) {
		SDL_Log("Couldn't create Vulkan surface");
		return NULL;
	}
	return vulkan_surface;
}

VkPhysicalDevice init_device(VkInstance vulkan_instance) {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(vulkan_instance, &device_count, NULL);
	if (device_count < 1) {
		SDL_Log("Failed to find GPUs with Vulkan support!");
		return NULL;
	}
	VkPhysicalDevice selected_device = NULL;
	VkPhysicalDevice *devices = calloc(sizeof(VkPhysicalDevice), device_count);
	vkEnumeratePhysicalDevices(vulkan_instance, &device_count, devices);
	for (int i = 0; i < device_count; i ++) {
		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceFeatures(devices[i], &device_features);
		if (device_features.geometryShader) {
			selected_device = devices[i];
			break;
		}
	}

	if (selected_device == NULL) {
		SDL_Log("No GPU with geometry shader support! will use first avaible!");
		if (device_count > 0) {
			selected_device = devices[0];
		}
	}
	if (selected_device == NULL) {
		SDL_Log("failed to find a suitable GPU!");
		return NULL;
	}
	free(devices);
	return selected_device;
}

void graphic_Queue(VkInstance vulkan_instance, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, int *graphics_queue_family_index, int *present_queue_family_index) {
	unsigned int queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, NULL);
	VkQueueFamilyProperties *queue_families = calloc(sizeof(VkQueueFamilyProperties), queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, queue_families);

	// Iterate over each queue to learn whether it supports presenting:
	VkBool32 *pSupportsPresent = (VkBool32 *)malloc(queue_family_count * sizeof(VkBool32));
	for (uint32_t i = 0; i < queue_family_count; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &pSupportsPresent[i]);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	*graphics_queue_family_index = -1;
	*present_queue_family_index = -1;
	for (uint32_t i = 0; i < queue_family_count; ++i) {
		if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (*graphics_queue_family_index == -1)
				*graphics_queue_family_index = i;
			if (pSupportsPresent[i] == VK_TRUE) {
				*graphics_queue_family_index = i;
				*present_queue_family_index = i;
				break;
			}
		}
	}

	if (*present_queue_family_index == -1) {
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (size_t i = 0; i < queue_family_count; ++i)
		    if (pSupportsPresent[i] == VK_TRUE) {
			    *present_queue_family_index = i;
			    break;
		    }
	}
	free(pSupportsPresent);
}

VkDevice init_virt_device(VkInstance vulkan_instance, VkPhysicalDevice physicalDevice, int graphic_queue_id) {
	VkDeviceQueueCreateInfo queueCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	    .queueFamilyIndex = graphic_queue_id,
	    .queueCount = 1
	};
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {0};
	VkDeviceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .pQueueCreateInfos = &queueCreateInfo,
	    .queueCreateInfoCount = 1,
	    .pEnabledFeatures = &deviceFeatures,
	    .enabledExtensionCount = 0,
	    .enabledLayerCount = 0
	};
	VkDevice logicalDevice = NULL;
	if (vkCreateDevice(physicalDevice, &createInfo, NULL, &logicalDevice) != VK_SUCCESS) {
		SDL_Log("failed to create logical device!");
		return NULL;
	}
	return logicalDevice;
}

int main(int argc, char *argv[])
{
	SDL_Window *window;                    // Declare a pointer
	int windowWidth = 640;
	int windowHeight = 480;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	// Create an application window with the following settings:
	window = SDL_CreateWindow(
		"An SDL2 window",                  // window title
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		windowWidth,                               // width, in pixels
		windowHeight,                               // height, in pixels
		0                                  // flags
	);

	// Check that the window was successfully created
	if (window == NULL) {
		// In the case that the window could not be made...
		SDL_Log("Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	static SDL_SysWMinfo sys_wm_info;
	SDL_VERSION(&sys_wm_info.version);
	if(!SDL_GetWindowWMInfo(window, &sys_wm_info)) {
		SDL_Log("Couldn't get window wm info");
		return 1;
	}

	VkInstance vulkan_instance = init_instance();
	VkPhysicalDevice phy_device = NULL;
	VkSurfaceKHR vulkan_surface = NULL;
	int graphics_queue_family_index = -1;
	int present_queue_family_index = -1;
	VkDevice logicalDevice = NULL;
	VkQueue graphicsQueue;
	if (vulkan_instance)
		vulkan_surface = init_surface(vulkan_instance, &sys_wm_info);

	if (vulkan_instance)
		phy_device = init_device(vulkan_instance);

	if (phy_device && vulkan_surface)
		graphic_Queue(vulkan_instance, phy_device, vulkan_surface, &graphics_queue_family_index, &present_queue_family_index);

	if (graphics_queue_family_index >= 0)
		logicalDevice = init_virt_device(vulkan_instance, phy_device, graphics_queue_family_index);
	else
		SDL_Log("No graphics queue!");

	if (logicalDevice)
		vkGetDeviceQueue(logicalDevice, graphics_queue_family_index, 0, &graphicsQueue);

	VkSwapchainKHR swapChain = NULL;
	VkImage *swapchainImages = NULL;
	VkImageView *swapchainViews = NULL;
	{
	    // Get the list of VkFormats that are supported:
	    uint32_t formatCount;
	    vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, vulkan_surface, &formatCount, NULL);
	    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	    vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, vulkan_surface, &formatCount, surfFormats);

	    VkFormat imageFormat;
	    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	    // the surface has no preferred format.  Otherwise, at least one
	    // supported format will be returned.
	    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	    } else {
		imageFormat = surfFormats[0].format;
	    }
	    free(surfFormats);

	    VkSurfaceCapabilitiesKHR surfCapabilities;

	    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_device, vulkan_surface, &surfCapabilities);

	    uint32_t presentModeCount;
	    vkGetPhysicalDeviceSurfacePresentModesKHR(phy_device, vulkan_surface, &presentModeCount, NULL);

	    VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));

	    vkGetPhysicalDeviceSurfacePresentModesKHR(phy_device, vulkan_surface, &presentModeCount, presentModes);

	    VkExtent2D swapchainExtent;
	    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
	    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = windowWidth;
		swapchainExtent.height = windowHeight;
		if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
		    swapchainExtent.width = surfCapabilities.minImageExtent.width;
		} else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
		    swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		}

		if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
		    swapchainExtent.height = surfCapabilities.minImageExtent.height;
		} else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
		    swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		}
	    } else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
	    }

	    // The FIFO present mode is guaranteed by the spec to be supported
	    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	    // Determine the number of VkImage's to use in the swap chain.
	    // We need to acquire only 1 presentable image at at time.
	    // Asking for minImageCount images ensures that we can acquire
	    // 1 presentable image as long as we present it before attempting
	    // to acquire another.
	    uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

	    VkSurfaceTransformFlagBitsKHR preTransform;
	    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	    } else {
		preTransform = surfCapabilities.currentTransform;
	    }

	    // Find a supported composite alpha mode - one of these is guaranteed to be set
	    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	    };
	    for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++) {
		if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
		    compositeAlpha = compositeAlphaFlags[i];
		    break;
		}
	    }

	    VkSwapchainCreateInfoKHR vulkan_swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.surface = vulkan_surface,
		.minImageCount = desiredNumberOfSwapChainImages,
		.imageFormat = imageFormat,
		.imageExtent.width = swapchainExtent.width,
		.imageExtent.height = swapchainExtent.height,
		.preTransform = preTransform,
		.compositeAlpha = compositeAlpha,
		.imageArrayLayers = 1,
		.presentMode = swapchainPresentMode,
		.oldSwapchain = VK_NULL_HANDLE,
		.clipped = 1,
		.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL
	    };
	    uint32_t queueFamilyIndices[2] = {(uint32_t)graphics_queue_family_index, (uint32_t)present_queue_family_index};
	    if (graphics_queue_family_index != present_queue_family_index) {
		// If the graphics and present queues are from different queue families,
		// we either have to explicitly transfer ownership of images between
		// the queues, or we have to create the swapchain with imageSharingMode
		// as VK_SHARING_MODE_CONCURRENT
		vulkan_swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		vulkan_swapchain_create_info.queueFamilyIndexCount = 2;
		vulkan_swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
	    }

	    vkCreateSwapchainKHR(logicalDevice, &vulkan_swapchain_create_info, NULL, &swapChain);

	    unsigned int swapchainImageCount;
	    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapchainImageCount, NULL);

	    VkImage *swapchainImages = (VkImage *)malloc(swapchainImageCount * sizeof(VkImage));
	    VkImageView *swapchainViews = (VkImageView *)malloc(swapchainImageCount * sizeof(VkImageView));
	    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapchainImageCount, swapchainImages);

	    for (uint32_t i = 0; i < swapchainImageCount; i++) {
		VkImageViewCreateInfo color_image_view = {
		    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		    .pNext = NULL,
		    .flags = 0,
		    .image = swapchainImages[i],
		    .viewType = VK_IMAGE_VIEW_TYPE_2D,
		    .format = imageFormat,
		    .components.r = VK_COMPONENT_SWIZZLE_R,
		    .components.g = VK_COMPONENT_SWIZZLE_G,
		    .components.b = VK_COMPONENT_SWIZZLE_B,
		    .components.a = VK_COMPONENT_SWIZZLE_A,
		    .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		    .subresourceRange.baseMipLevel = 0,
		    .subresourceRange.levelCount = 1,
		    .subresourceRange.baseArrayLayer = 0,
		    .subresourceRange.layerCount = 1
		};

		vkCreateImageView(logicalDevice, &color_image_view, NULL, &swapchainViews[i]);
	    }

	}

	// The window is open: could enter program loop here (see SDL_PollEvent())
	SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example

	if (swapchainImages)
		free(swapchainImages);

	if (swapChain)
		vkDestroySwapchainKHR(logicalDevice, swapChain, NULL);

	if (logicalDevice)
		vkDestroyDevice(logicalDevice, NULL);

	if (vulkan_surface)
		vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, NULL);

	if (vulkan_instance)
		vkDestroyInstance(vulkan_instance, NULL);

	// Close and destroy the window
	SDL_DestroyWindow(window);

	// Clean up
	SDL_Quit();

	return 0;
}
