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

VkResult cpu_CreateDescriptorSetLayout(
	VkDevice                                    _device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorSetLayout*                      pSetLayout)
{
	CPU_FROM_HANDLE(cpu_device, device, _device);
	struct cpu_descriptor_set_layout *set_layout;

	assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);

	uint32_t max_binding = 0;
	uint32_t immutable_sampler_count = 0;
	for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
		max_binding = MAX2(max_binding, pCreateInfo->pBindings[j].binding);
		if (pCreateInfo->pBindings[j].pImmutableSamplers)
			immutable_sampler_count += pCreateInfo->pBindings[j].descriptorCount;
	}

	size_t size = sizeof(struct cpu_descriptor_set_layout) +
		(max_binding + 1) * sizeof(set_layout->binding[0]) +
		immutable_sampler_count * sizeof(struct cpu_sampler *);

	set_layout = vk_alloc2(&device->alloc, pAllocator, size, 8,
				 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if (!set_layout)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	/* We just allocate all the samplers at the end of the struct */
	struct cpu_sampler **samplers =
		(struct cpu_sampler **)&set_layout->binding[max_binding + 1];

	set_layout->binding_count = max_binding + 1;
	set_layout->shader_stages = 0;
	set_layout->size = 0;

	memset(set_layout->binding, 0, size - sizeof(struct cpu_descriptor_set_layout));

	uint32_t buffer_count = 0;
	uint32_t dynamic_offset_count = 0;

	for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
		const VkDescriptorSetLayoutBinding *binding = &pCreateInfo->pBindings[j];
		uint32_t b = binding->binding;
		uint32_t alignment;

		switch (binding->descriptorType) {
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			set_layout->binding[b].dynamic_offset_count = 1;
			set_layout->dynamic_shader_stages |= binding->stageFlags;
			set_layout->binding[b].size = 0;
			set_layout->binding[b].buffer_count = 1;
			alignment = 1;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			set_layout->binding[b].size = 16;
			set_layout->binding[b].buffer_count = 1;
			alignment = 16;
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			/* main descriptor + fmask descriptor */
			set_layout->binding[b].size = 64;
			set_layout->binding[b].buffer_count = 1;
			alignment = 32;
			break;
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			/* main descriptor + fmask descriptor + sampler */
			set_layout->binding[b].size = 96;
			set_layout->binding[b].buffer_count = 1;
			alignment = 32;
			break;
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			set_layout->binding[b].size = 16;
			alignment = 16;
			break;
		default:
			unreachable("unknown descriptor type\n");
			break;
		}

		set_layout->size = align(set_layout->size, alignment);
		assert(binding->descriptorCount > 0);
		set_layout->binding[b].type = binding->descriptorType;
		set_layout->binding[b].array_size = binding->descriptorCount;
		set_layout->binding[b].offset = set_layout->size;
		set_layout->binding[b].buffer_offset = buffer_count;
		set_layout->binding[b].dynamic_offset_offset = dynamic_offset_count;

		set_layout->size += binding->descriptorCount * set_layout->binding[b].size;
		buffer_count += binding->descriptorCount * set_layout->binding[b].buffer_count;
		dynamic_offset_count += binding->descriptorCount *
			set_layout->binding[b].dynamic_offset_count;


		if (binding->pImmutableSamplers) {
			set_layout->binding[b].immutable_samplers = samplers;
			samplers += binding->descriptorCount;

			for (uint32_t i = 0; i < binding->descriptorCount; i++)
				set_layout->binding[b].immutable_samplers[i] =
					cpu_sampler_from_handle(binding->pImmutableSamplers[i]);
		} else {
			set_layout->binding[b].immutable_samplers = NULL;
		}

		set_layout->shader_stages |= binding->stageFlags;
	}

	set_layout->buffer_count = buffer_count;
	set_layout->dynamic_offset_count = dynamic_offset_count;

	*pSetLayout = cpu_descriptor_set_layout_to_handle(set_layout);

	return VK_SUCCESS;
}

void cpu_DestroyDescriptorSetLayout(
	VkDevice                                    _device,
	VkDescriptorSetLayout                       _set_layout,
	const VkAllocationCallbacks*                pAllocator)
{
	CPU_FROM_HANDLE(cpu_device, device, _device);
	CPU_FROM_HANDLE(cpu_descriptor_set_layout, set_layout, _set_layout);

	if (!set_layout)
		return;

	vk_free2(&device->alloc, pAllocator, set_layout);
}

VkResult cpu_CreatePipelineLayout(
	VkDevice                                    _device,
	const VkPipelineLayoutCreateInfo*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkPipelineLayout*                           pPipelineLayout)
{
	// TODO: return real support
	return VK_INCOMPLETE;
}

void cpu_DestroyPipelineLayout(
	VkDevice                                    _device,
	VkPipelineLayout                            _pipelineLayout,
	const VkAllocationCallbacks*                pAllocator)
{
	// TODO: implement
}
