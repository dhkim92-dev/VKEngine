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
		VkImageView view;
		VkDeviceMemory memory;
		VkSampler sampler;

		explicit Image(Context *_context, int height, int width, VkImageUsageFlags usage, VkMemoryPropertyFlags flags){
			context = _context;
			device = VkDevice(*context);
		}

		VkDeviceSize size = 0;
		VkMemoryRequirements memory_requirements;
		VkDescriptorImageInfo descriptor;
	};
}

#endif