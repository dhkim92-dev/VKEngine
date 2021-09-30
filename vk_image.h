#ifndef __VK_IMAGE_H__
#define __VK_IMAGE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "vk_context.h"
#include "vk_utils.h"
#include "vk_infos.h"

namespace VKEngine{


struct ImageAttachment{
	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
	VkFormat format;
};

struct Image{
	private :
	Context *context = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	public : 
	VkImage image;
	VkImageLayout image_layout;
	VkImageView view;
	VkDeviceMemory memory;
	VkSampler sampler;
	VkDeviceSize size = 0;
	VkMemoryRequirements memory_requirements;
	VkDescriptorImageInfo descriptor;
	VkMemoryPropertyFlags mem_properties;
	VkImageUsageFlags usages;

	uint32_t height, width, channels;
	uint32_t mip_level, nr_layers;

	Image();
	Image(Context *ctx, 
			uint32_t h, uint32_t w, uint32_t ch,
			VkImageUsageFlags usages, VkMemoryPropertyFlags mem_flags);
	void create(Context *ctx, 
				uint32_t h, uint32_t w, uint32_t ch, 
				VkImageUsageFlags usages, VkMemoryPropertyFlags mem_flags);
	void bind();
	VkImage createImage();
	VkDeviceMemory alloc(uint32_t mem_size);
	VkSampler createSampler();
	VkImageView createImageView();
	void transitionState();
};


}

#endif