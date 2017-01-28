#include <X11/Xlib-xcb.h>
#include <X11/xshmfence.h>
#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>

#include "cpu_private.h"

VkBool32 cpu_GetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id)
{
	// TODO: return real support
	return false;
}

VkBool32 cpu_GetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID)
{
	// TODO: return real support
	return false;
}

#ifdef VK_USE_PLATFORM_XCB_KHR
VkResult cpu_CreateXcbSurfaceKHR(
    VkInstance                                  _instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
	// TODO: return real support
	return VK_INCOMPLETE;
}
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
VkResult cpu_CreateXlibSurfaceKHR(
    VkInstance                                  _instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
	// TODO: return real support
	return VK_INCOMPLETE;
}
#endif
