/*
 * Copyright © 2017 Denis Pauk
 *
 * based in part on anv and radv drivers which is:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Bas Nieuwenhuizen
 * Copyright © 2016 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include "cpu_private.h"
#include "vk_alloc.h"

VkResult cpu_CreateShaderModule(
	VkDevice                                    _device,
	const VkShaderModuleCreateInfo*             pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkShaderModule*                             pShaderModule)
{
	CPU_FROM_HANDLE(cpu_device, device, _device);
	struct cpu_shader_module *module;

	assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
	assert(pCreateInfo->flags == 0);

	module = vk_alloc2(&device->alloc, pAllocator,
			     sizeof(*module) + pCreateInfo->codeSize, 8,
			     VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if (module == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	module->nir = NULL;
	module->size = pCreateInfo->codeSize;
	memcpy(module->data, pCreateInfo->pCode, module->size);

	// TODO: compute sha1 for module->sha1

	*pShaderModule = cpu_shader_module_to_handle(module);

	return VK_SUCCESS;
}

void cpu_DestroyShaderModule(
	VkDevice                                    _device,
	VkShaderModule                              _module,
	const VkAllocationCallbacks*                pAllocator)
{
	CPU_FROM_HANDLE(cpu_device, device, _device);
	CPU_FROM_HANDLE(cpu_shader_module, module, _module);

	if (!module)
		return;

	vk_free2(&device->alloc, pAllocator, module);
}
