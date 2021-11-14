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
void *data = nullptr;
public : 
VkImage image = VK_NULL_HANDLE;
VkDeviceMemory memory = VK_NULL_HANDLE;
VkImageType type;
VkMemoryPropertyFlags memory_properties;
VkFormat format;
VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
VkImageUsageFlags usages;
VkImageView view = VK_NULL_HANDLE;
VkSampler sampler = VK_NULL_HANDLE;
VkMemoryRequirements memory_requirements ; 
VkDescriptorImageInfo descriptor;
VkSampleCountFlagBits nr_samples;
VkImageTiling tiling;
uint32_t height, width, channels;
uint32_t mip_level, nr_layers;
VkDeviceSize sz_memory = 0;

Image();
Image(Context *ctx);
~Image();
void create(Context *ctx);
VkResult createImage(uint32_t w, uint32_t h, uint32_t ch,
					VkImageType img_type, 
					VkImageUsageFlags img_usage,	
					VkFormat img_format, VkImageTiling img_tiling, 
					VkSampleCountFlagBits sample, uint32_t mip_level, uint32_t array_layer);

VkResult alloc(VkDeviceSize sz_mem, VkMemoryPropertyFlags flags);
void setMemory(VkDeviceMemory mem);
VkResult bind(VkDeviceSize offset);
VkResult createImageView(VkImageViewType view_type, VkImageSubresourceRange range);
VkResult createSampler(VkSamplerCreateInfo *info);
void setupDescriptor();
VkResult map(VkDeviceSize offset, VkDeviceSize size=VK_WHOLE_SIZE);
void unmap();
VkResult invalidate(VkDeviceSize offset, VkDeviceSize size = VK_WHOLE_SIZE);
VkResult flush(VkDeviceSize offset, VkDeviceSize = VK_WHOLE_SIZE);
void destroy();

void copyFrom(void *src, VkDeviceSize size);
void copyTo(void *dst, VkDeviceSize size);
void setLayout(VkCommandBuffer command, VkImageAspectFlags aspect_mask, 
			   VkImageLayout old_layout, VkImageLayout new_layout,
			   VkImageSubresourceRange subresource_range,
			   VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
			   VkAccessFlags src_mask, VkAccessFlags dst_mask);
VkImageMemoryBarrier barrier(
	VkImageLayout old_layout, VkImageLayout new_layout, 
	VkImageSubresourceRange range, 
	uint32_t src_queue_index = VK_QUEUE_FAMILY_IGNORED, uint32_t dst_queue_index = VK_QUEUE_FAMILY_IGNORED);

	VkImage getImage() const{
		return this->image;
	}

	VkImageView getView() const{
		return this->view;
	}

	VkSampler getSampler() const{
		return this->sampler;
	}

	VkImageLayout getLayout() const {
		return this->layout;
	}
};


}

#endif