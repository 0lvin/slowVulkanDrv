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

void* init_device(VkInstance vulkan_instance) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vulkan_instance, &deviceCount, NULL);
	if (deviceCount < 1) {
		SDL_Log("Failed to find GPUs with Vulkan support!");
		return NULL;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	SDL_Window *window;                    // Declare a pointer

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	// Create an application window with the following settings:
	window = SDL_CreateWindow(
		"An SDL2 window",                  // window title
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		640,                               // width, in pixels
		480,                               // height, in pixels
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
	if (vulkan_instance) {
		VkSurfaceKHR vulkan_surface = init_surface(vulkan_instance, &sys_wm_info);
	}
	// The window is open: could enter program loop here (see SDL_PollEvent())
	SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example

	// Close and destroy the window
	SDL_DestroyWindow(window);

	// Clean up
	SDL_Quit();

	return 0;
}
