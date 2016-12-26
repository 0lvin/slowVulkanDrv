#ifndef SLOW_PRIVATE_H
#define SLOW_PRIVATE_H

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "slow_entrypoints.h"

void *slow_lookup_entrypoint(const char *name);

/* Compute the size of an array */
#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

#endif /* SLOW_PRIVATE_H */
