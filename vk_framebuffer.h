#ifndef __VK_FRAMEABUFFER_H__
#define __VK_FRAMEBUFFER_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <algorithm>
#include <cassert>
#include "vk_context.h"
#include "vk_swapchain.h"
#include "vk_infos.h"
#include "vk_utils.h"

using namespace std;

namespace VKEngine{
	struct FramebufferAttachment{
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkFormat format;
		VkImageSubresourceRange subresource_range;
		VkAttachmentDescription description;

		bool hasDepth(){
			vector<VkFormat> formats = {
				VK_FORMAT_D16_UNORM,
				VK_FORMAT_X8_D24_UNORM_PACK32,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT
			};
			return (find(formats.begin(), formats.end(), format) != formats.end());
		};

		bool hasStencil(){
			vector<VkFormat> formats = {
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT
			};
			return (find(formats.begin(), formats.end(), format) != formats.end());
		};

		bool isDepthStencil(){
			return (hasDepth() || hasStencil());
		}
	};
	
	struct AttachmentCreateInfo{
		uint32_t width, height;
		uint32_t nr_layers;
		VkFormat format;
		VkImageUsageFlags usage;
		VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT;
	};

	struct Framebuffer{
		private :
		Context *context = nullptr; 
		VkPhysicalDevice gpu; 
		VkDevice device;
		public : 
		uint32_t height, width;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkRenderPass render_pass = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		vector<FramebufferAttachment> attachments;

		explicit Framebuffer(Context *context);
		~Framebuffer();
		uint32_t addAttachment(AttachmentCreateInfo info);
		void createSampler(VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode address_mode);
		void createRenderPass();
		void createFramebuffer();
		void destroy();
	};
}

#endif