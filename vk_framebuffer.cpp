#ifndef __VK_FRAMEBUFFER_CPP__
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
		image_CI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_CI.mipLevels = 1;

		VkMemoryAllocateInfo mem_AI = infos::memoryAllocateInfo();
		VkMemoryRequirements mem_reqs;

		LOG("Framebuffer::addAttachment, vkCreateImage\n");
		VK_CHECK_RESULT(vkCreateImage(device, &image_CI, nullptr, &fb_attachment.image)); 
		LOG("Framebuffer::addAttachment, vkCreateImage\n");

		LOG("memory allocation\n");
		vkGetImageMemoryRequirements(device, fb_attachment.image, &mem_reqs);
		mem_AI.allocationSize = mem_reqs.size;
		mem_AI.memoryTypeIndex = getMemoryType(gpu, device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT( vkAllocateMemory(device, &mem_AI, nullptr, &fb_attachment.memory) );
		VK_CHECK_RESULT( vkBindImageMemory(device, fb_attachment.image, fb_attachment.memory, 0) );
		LOG("memory allocation\n");
		fb_attachment.subresource_range = {};
		fb_attachment.subresource_range.aspectMask = aspect_mask;
		fb_attachment.subresource_range.levelCount = 1;
		fb_attachment.subresource_range.layerCount = info.nr_layers;
		LOG("viewCreate\n");
		VkImageViewCreateInfo image_view_CI = infos::imageViewCreateInfo();
		image_view_CI.viewType = (info.nr_layers==1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		image_view_CI.format = info.format;
		image_view_CI.subresourceRange = fb_attachment.subresource_range;
		image_view_CI.subresourceRange.aspectMask = fb_attachment.hasDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : aspect_mask;
		image_view_CI.image = fb_attachment.image;
		VK_CHECK_RESULT( vkCreateImageView(device, &image_view_CI, nullptr, &fb_attachment.view) );

		LOG("viewCreate\n");
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

	void Framebuffer::createSampler(VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode address_mode)
	{
		VkSamplerCreateInfo sampler_CI = infos::samplerCreateInfo();
		sampler_CI.magFilter = mag_filter;
		sampler_CI.minFilter = min_filter;
		sampler_CI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_CI.addressModeU = address_mode;
		sampler_CI.addressModeV = address_mode;
		sampler_CI.addressModeW = address_mode;
		sampler_CI.mipLodBias = 0.0f;
		sampler_CI.maxAnisotropy = 1.0f;
		sampler_CI.minLod = 0.0f;
		sampler_CI.maxLod = 1.0f;
		sampler_CI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		
		VK_CHECK_RESULT(vkCreateSampler(device, &sampler_CI, nullptr, &sampler));
	}

	void Framebuffer::createRenderPass(){
		vector<VkAttachmentDescription> attachment_descriptions;
		vector<VkAttachmentReference> color_refers;
		VkAttachmentReference depth_refer = {};
		bool has_depth = false;
		bool has_color = false;
		uint32_t attachment_idx = 0;
		VkSubpassDescription subpass_desc = {};

		for(FramebufferAttachment attachment : attachments){
			attachment_descriptions.push_back(attachment.description);
		}

		for(FramebufferAttachment attachment : attachments){
			if(attachment.isDepthStencil()){
				assert(!has_depth);
				depth_refer.attachment = attachment_idx;
				depth_refer.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				has_depth = true;
			}else{
				color_refers.push_back( {attachment_idx, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} );
				has_color = true;
			}
			attachment_idx++;
		}

		subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		if(has_color){
			subpass_desc.pColorAttachments = color_refers.data();
			subpass_desc.colorAttachmentCount = static_cast<uint32_t>(color_refers.size());
		}

		if(has_depth){
			subpass_desc.pDepthStencilAttachment = &depth_refer;
		}

		vector<VkSubpassDependency> subpass_dependencies(2);

		subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependencies[0].dstSubpass = 0;
		subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		
		subpass_dependencies[1].srcSubpass = 0;
		subpass_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass_dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpass_dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo render_pass_CI = infos::renderPassCreateInfo();
		render_pass_CI.pAttachments = attachment_descriptions.data();
		render_pass_CI.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
		render_pass_CI.subpassCount = 1;
		render_pass_CI.pSubpasses = &subpass_desc;
		render_pass_CI.dependencyCount = 2;
		render_pass_CI.pDependencies = subpass_dependencies.data();
		VK_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_CI, nullptr, &render_pass));
	}		

	void Framebuffer::createFramebuffer(){
		LOG("Framebuffer::createFramebuffer()\n");
		VkFramebufferCreateInfo framebuffer_CI = {};
		vector<VkImageView> attachment_views;
		uint32_t max_layers = 0;
		
		for(FramebufferAttachment attachment : attachments){
			attachment_views.push_back(attachment.view);
			max_layers = ( attachment.subresource_range.layerCount > max_layers ) ? attachment.subresource_range.layerCount : max_layers; 
		}

		framebuffer_CI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_CI.renderPass = render_pass;
		framebuffer_CI.pAttachments = attachment_views.data();
		framebuffer_CI.attachmentCount = static_cast<uint32_t>( attachment_views.size() );
		framebuffer_CI.width = width;
		framebuffer_CI.height = height;
		framebuffer_CI.layers = max_layers;
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebuffer_CI, nullptr, &framebuffer));
	}

	void Framebuffer::destroy(){
		assert(device);
		
		for(FramebufferAttachment attachment : attachments){
			vkDestroyImage(device, attachment.image, nullptr);
			vkDestroyImageView(device, attachment.view, nullptr);
			vkFreeMemory(device, attachment.memory, nullptr);
		}
		if(sampler) vkDestroySampler(device, sampler, nullptr);
		if(render_pass)	vkDestroyRenderPass(device, render_pass, nullptr);
		if(framebuffer)	vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

#endif