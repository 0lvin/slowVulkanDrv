#ifndef SLOW_PRIVATE_H
#define SLOW_PRIVATE_H

#include <vulkan/vulkan.h>
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

#endif /* SLOW_PRIVATE_H */
