#ifndef SLOW_PRIVATE_H
#define SLOW_PRIVATE_H

#include <stdbool.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "slow_entrypoints.h"
#include "util/macros.h"

#define MAX_PUSH_CONSTANTS_SIZE 128
#define MAX_SETS         8
#define MAX_VIEWPORTS   16
#define MAX_RTS          8

void *slow_lookup_entrypoint(const char *name);

#define typed_memcpy(dest, src, count) ({				\
			static_assert(sizeof(*src) == sizeof(*dest), ""); \
			memcpy((dest), (src), (count) * sizeof(*(src))); \
		})

struct slow_instance {
	VK_LOADER_DATA			_loader_data;

	VkAllocationCallbacks		alloc;

	uint32_t			apiVersion;
	int				physicalDeviceCount;
	VkPhysicalDevice*		physicalDevices;
};

#define SLOW_DEFINE_HANDLE_CASTS(__slow_type, __VkType)		\
								\
	static inline struct __slow_type *			\
	__slow_type ## _from_handle(__VkType _handle)		\
	{							\
		return (struct __slow_type *) _handle;		\
	}							\
								\
	static inline __VkType					\
	__slow_type ## _to_handle(struct __slow_type *_obj)	\
	{							\
		return (__VkType) _obj;				\
	}

#define SLOW_FROM_HANDLE(__slow_type, __name, __handle)			\
	struct __slow_type *__name = __slow_type ## _from_handle(__handle)

SLOW_DEFINE_HANDLE_CASTS(slow_instance, VkInstance)

#endif /* SLOW_PRIVATE_H */
