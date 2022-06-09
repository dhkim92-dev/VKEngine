#ifndef __VK_DESCRIPTORS_CPP__
#define __VK_DESCRIPTORS_CPP__

#include "vk_descriptors.h"

namespace VKEngine
{
// DescriptorSetLayout Builder
VkResult DescriptorSetLayoutBuilder::build(Context* ctx, VkDescriptorSetLayout *dst, vector<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayoutCreateFlags flags,void *pNext)
{
	return build(ctx, dst, bindings.data(), bindings.size(), flags, pNext);
}


VkResult DescriptorSetLayoutBuilder::build(Context *ctx, VkDescriptorSetLayout *dst, VkDescriptorSetLayoutBinding* bindings, size_t count,  VkDescriptorSetLayoutCreateFlags flags, void *pNext)
{
	VkDescriptorSetLayoutCreateInfo info={};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.flags = flags;
	info.bindingCount = count;
	info.pBindings = bindings;
	info.pNext = pNext;

	return vkCreateDescriptorSetLayout(ctx->getDevice(), &info, nullptr, dst);

}

VkDescriptorSetLayoutBinding DescriptorSetLayoutBuilder::bind(uint32_t id, VkDescriptorType type, uint32_t count, VkShaderStageFlags flags, VkSampler *sampler)
{
	VkDescriptorSetLayoutBinding binding={};
	binding.binding = id;
	binding.descriptorCount = count;
	binding.descriptorType = type;
	binding.stageFlags = flags;
	binding.pImmutableSamplers=sampler;

	return binding;
}

void DescriptorSetLayoutBuilder::destroy(Context *ctx, VkDescriptorSetLayout *layout)
{
	if(*layout != VK_NULL_HANDLE){
		vkDestroyDescriptorSetLayout(ctx->getDevice(), *layout, nullptr);
		*layout = VK_NULL_HANDLE;
	}
}
// DescriptorSetBuilder

DescriptorSetBuilder::DescriptorSetBuilder(Context *ctx) : ctx(ctx){}
DescriptorSetBuilder::~DescriptorSetBuilder()
{
	if(pool != VK_NULL_HANDLE){
		vkDestroyDescriptorPool(ctx->getDevice(), pool, nullptr);
		pool = VK_NULL_HANDLE;
	}
}

VkDescriptorPoolSize DescriptorSetBuilder::createDescriptorPoolSize(VkDescriptorType type, uint32_t count)
{
	VkDescriptorPoolSize pool_size = {};
	pool_size.type = type;
	pool_size.descriptorCount = count;
	return pool_size;
}

VkResult DescriptorSetBuilder::setDescriptorPool(vector<VkDescriptorPoolSize> pool_sizes, uint32_t max_sets, VkDescriptorPoolCreateFlags flags, void *pNext)
{
	return setDescriptorPool(pool_sizes.data(), pool_sizes.size(), max_sets, flags, pNext);
}

VkResult DescriptorSetBuilder::setDescriptorPool(VkDescriptorPoolSize * pool_sizes, size_t count,  uint32_t max_sets, VkDescriptorPoolCreateFlags flags, void *pNext)
{
	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = flags;
	info.poolSizeCount = count;
	info.pPoolSizes = pool_sizes;
	info.pNext = pNext;
	info.maxSets = max_sets;
	return vkCreateDescriptorPool(ctx->getDevice(), &info, nullptr, &pool);
}

VkResult DescriptorSetBuilder::build(VkDescriptorSet *dst, VkDescriptorSetLayout* layouts, int count, void *pNext)
{
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.pSetLayouts=layouts;
	info.descriptorSetCount = count;
	info.descriptorPool = pool;
	info.pNext = pNext;

	return vkAllocateDescriptorSets(ctx->getDevice(), nullptr, dst);
}

VkResult DescriptorSetBuilder::free(VkDescriptorSet *sets, uint32_t count){
	VkResult res = VK_SUCCESS;
	VkDevice device = ctx->getDevice();
	res = vkFreeDescriptorSets(device, pool, count ,sets);
	for(int i = 0 ; i < count ; i++){
		*(sets+i) = VK_NULL_HANDLE;
	}
	return res;
}


//DescriptorReadWrite

VkWriteDescriptorSet DescriptorWriter::writeBuffer(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, Buffer *buffer, uint32_t descriptor_count)
{
	VkWriteDescriptorSet info = {};
	info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	info.descriptorCount = descriptor_count;
	info.descriptorType = type;
	info.dstBinding = binding;
	info.dstSet = set;
	info.pBufferInfo = buffer->getDescriptorInfoPtr();
	info.pNext = nullptr;
	return info;
}

VkWriteDescriptorSet writeImage(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, Image *image, uint32_t descriptor_count)
{
	VkWriteDescriptorSet info = {};
	info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	info.descriptorCount = descriptor_count;
	info.descriptorType = type;
	info.dstBinding = binding;
	info.dstSet = set;
	info.pImageInfo = image->getDescriptorInfoPtr();
	info.pNext = nullptr;
	return info;
}

void DescriptorWriter::update(Context *ctx, VkWriteDescriptorSet *writes, uint32_t writes_count)
{
	vkUpdateDescriptorSets(ctx->getDevice(), writes_count, writes, 0, nullptr);
}

void DescriptorWriter::copy(Context *ctx,  VkWriteDescriptorSet *writes_descriptorset, uint32_t writes_count, VkCopyDescriptorSet *copy_descriptorsets, uint32_t copy_count)
{
	vkUpdateDescriptorSets(ctx->getDevice(), writes_count, writes_descriptorset, copy_count, copy_descriptorsets);
}

}

#endif 