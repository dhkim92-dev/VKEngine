#ifndef __VK_FRAMEABUFFER_CPP__
#define __VK_FRAMEBUFFER_CPP__

#include "vk_framebuffer.h"

using namespace std;

namespace VKEngine{
	Framebuffer::Framebuffer(Context *context) : gpu(VkPhysicalDevice(*context)), device(VkDevice(*context)){

	};
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
		mem_AI.memoryTypeIndex = getMemoryType(gpu, device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT( vkAllocateMemory(device, &mem_AI, nullptr, &fb_attachment.memory) );
		VK_CHECK_RESULT( vkBindImageMemory(device, fb_attachment.image, fb_attachment.memory, 0) );

		fb_attachment.subresource_range = {};
		fb_attachment.subresource_range.aspectMask = aspect_mask;
		fb_attachment.subresource_range.levelCount = 1;
		fb_attachment.subresource_range.layerCount = info.nr_layers;

		VkImageViewCreateInfo image_view_CI = infos::imageViewCreateInfo();
		image_view_CI.viewType = (info.nr_layers==1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		image_view_CI.format = info.format;
		image_view_CI.subresourceRange = fb_attachment.subresource_range;
		image_view_CI.subresourceRange.aspectMask = fb_attachment.hasDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : aspect_mask;
		image_view_CI.image = fb_attachment.image;
		VK_CHECK_RESULT( vkCreateImageView(device, &image_view_CI, nullptr, &fb_attachment.view) );

		fb_attachment.description = {};
		fb_attachment.description.samples = info.sample_count;
		fb_attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		fb_attachment.description.storeOp = (info.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
		fb_attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		fb_attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		fb_attachment.description.format = info.format;
		fb_attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if(fb_attachment.isDepthStencil()){
			fb_attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}else{
			fb_attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		attachments.push_back(fb_attachment);

		return static_cast<uint32_t>(attachments.size() - 1);
	}
}

#endif