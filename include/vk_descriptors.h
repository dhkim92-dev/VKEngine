#ifndef __VK_DESCRIPTORSET_H__
#define __VK_DESCRIPTORSET_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "vk_context.h"
#include "vk_utils.h"
#include "vk_buffer.h"
#include "vk_image.h"

using namespace std;

namespace VKEngine{

class DescriptorWriter{
	public: 
	static VkWriteDescriptorSet writeBuffer(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, Buffer *buffer, uint32_t descriptor_count =1);
	static VkWriteDescriptorSet writeImage(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, Image *image, uint32_t descriptor_count = 1);
	static void update(Context *ctx, VkWriteDescriptorSet *writes, uint32_t writes_count);
	static void copy(Context *ctx, VkWriteDescriptorSet *writes_descriptorset, uint32_t writes_count, VkCopyDescriptorSet *copy_descriptorsets, uint32_t copy_count);
};

class DescriptorSetLayoutBuilder
{
	public:
	static VkResult build(Context *ctx, VkDescriptorSetLayout *dst, vector<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayoutCreateFlags flags = 0 ,void *pNext=nullptr);
	static VkResult build(Context *ctx, VkDescriptorSetLayout *dst, VkDescriptorSetLayoutBinding* bindings, size_t count,  VkDescriptorSetLayoutCreateFlags flags = 0 ,void *pNext=nullptr);
	static VkDescriptorSetLayoutBinding bind(uint32_t id, VkDescriptorType type, uint32_t count, VkShaderStageFlags flags, VkSampler *sampler=nullptr);
	static void destroy(Context *ctx, VkDescriptorSetLayout *layout);
};

class DescriptorSetBuilder
{
	private:
	Context *ctx;
	VkDescriptorPool pool=VK_NULL_HANDLE;

	public:
	explicit DescriptorSetBuilder(Context *ctx);
	~DescriptorSetBuilder();

	static VkDescriptorPoolSize createDescriptorPoolSize(VkDescriptorType type, uint32_t count=1);
	VkResult setDescriptorPool(vector<VkDescriptorPoolSize> pool_sizes, uint32_t max_sets, VkDescriptorPoolCreateFlags flags=VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, void *pNext=nullptr);
	VkResult setDescriptorPool(VkDescriptorPoolSize * pool_sizes, size_t count,  uint32_t max_sets, VkDescriptorPoolCreateFlags flags=VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, void *pNext=nullptr);
	VkResult build(VkDescriptorSet *dst, VkDescriptorSetLayout* layouts, int count, void *pNext=nullptr);
	VkResult free(VkDescriptorSet *sets, uint32_t count);
	VkDescriptorPool getPool() const {return pool;}
};

}
#endif