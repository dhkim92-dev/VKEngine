#ifndef __VK_FRAMEABUFFER_CPP__
#define __VK_FRAMEBUFFER_CPP__

#include "vk_framebuffer.h"

using namespace std;

namespace VKEngine{
	Framebuffer::Framebuffer(Context *context) : gpu(VkPhysicalDevice(*context)), device(VkDevice(*context)){};
	Framebuffer::~Framebuffer(){
		destroy();
	}
	
	uint32_t Framebuffer::addAttachment(AttachmentCreateInfo info){
		FramebufferAttachment fb_attachment = {};
		fb_attachment.format = info.format;
		VkImageAspectFlags aspect_mask = 0;

		if(info.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT){
			aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		if(info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT){
			if(fb_attachment.hasDepth()){
				aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}

			if(fb_attachment.hasStencil()){
				aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

		assert(aspect_mask > 0);


		VkImageCreateInfo image_CI = infos::imageCreateInfo();
		image_CI.format = fb_attachment.format;
		image_CI.imageType = VK_IMAGE_TYPE_2D;
		image_CI.extent.height = info.height;
		image_CI.extent.width = info.width;
		image_CI.extent.depth = 1;
		image_CI.arrayLayers = info.nr_layers;
		image_CI.samples = info.sample_count;
		image_CI.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_CI.usage = info.usage;

		VkMemoryAllocateInfo mem_AI = infos::memoryAllocateInfo();
		VkMemoryRequirements mem_reqs;

		VK_CHECK_RESULT(vkCreateImage(device, &image_CI, nullptr, &fb_attachment.image)); 
		vkGetImageMemoryRequirements(device, fb_attachment.image, &mem_reqs);
		mem_AI.allocationSize = mem_reqs.size;
		
	}
}

#endif