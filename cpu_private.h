#ifndef CPU_PRIVATE_H
#define CPU_PRIVATE_H

#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR

#include <stdbool.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "cpu_entrypoints.h"
#include "util/macros.h"

#define MAX_PUSH_CONSTANTS_SIZE 128
#define MAX_SETS         8
#define MAX_VIEWPORTS   16
#define MAX_RTS          8

void *cpu_lookup_entrypoint(const char *name);

#define typed_memcpy(dest, src, count) ({				\
			static_assert(sizeof(*src) == sizeof(*dest), ""); \
			memcpy((dest), (src), (count) * sizeof(*(src))); \
		})

struct cpu_instance {
	VK_LOADER_DATA			_loader_data;

	VkAllocationCallbacks		alloc;

	uint32_t			apiVersion;
	int				physicalDeviceCount;
	VkPhysicalDevice		physicalDevices[16];
};


struct cpu_physical_device {
	VK_LOADER_DATA			_loader_data;

	struct cpu_instance *		instance;
};

struct cpu_device {
	VK_LOADER_DATA			_loader_data;

	VkAllocationCallbacks		alloc;

	struct cpu_instance *		instance;
};

struct cpu_device_memory {
	uint32_t			type_index;
	VkDeviceSize			map_size;
	void *				map;
};

struct cpu_buffer {
	struct cpu_device *	device;
	VkDeviceSize		size;

	VkBufferUsageFlags	usage;

	/* Set when bound */
	void *			bo;
	VkDeviceSize		offset;
};

#define CPU_DEFINE_HANDLE_CASTS(__cpu_type, __VkType)		\
								\
	static inline struct __cpu_type *			\
	__cpu_type ## _from_handle(__VkType _handle)		\
	{							\
		return (struct __cpu_type *) _handle;		\
	}							\
								\
	static inline __VkType					\
	__cpu_type ## _to_handle(struct __cpu_type *_obj)	\
	{							\
		return (__VkType) _obj;				\
	}

#define CPU_DEFINE_NONDISP_HANDLE_CASTS(__cpu_type, __VkType)		\
									\
	static inline struct __cpu_type *				\
	__cpu_type ## _from_handle(__VkType _handle)			\
	{								\
		return (struct __cpu_type *)(uintptr_t) _handle;	\
	}								\
									\
	static inline __VkType						\
	__cpu_type ## _to_handle(struct __cpu_type *_obj)		\
	{								\
		return (__VkType)(uintptr_t) _obj;			\
	}

#define CPU_FROM_HANDLE(__cpu_type, __name, __handle)			\
	struct __cpu_type *__name = __cpu_type ## _from_handle(__handle)

CPU_DEFINE_HANDLE_CASTS(cpu_device, VkDevice)
CPU_DEFINE_HANDLE_CASTS(cpu_instance, VkInstance)
CPU_DEFINE_HANDLE_CASTS(cpu_physical_device, VkPhysicalDevice)

CPU_DEFINE_NONDISP_HANDLE_CASTS(cpu_buffer, VkBuffer)
CPU_DEFINE_NONDISP_HANDLE_CASTS(cpu_device_memory, VkDeviceMemory)

#endif /* CPU_PRIVATE_H */
