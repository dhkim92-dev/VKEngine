#ifndef __VK_SWAPCHAIN_CPP__
#define __VK_SWAPCHAIN_CPP__

#include "vk_swapchain.h"

using namespace std;

namespace VKEngine{

	void SwapChain::connect(Engine *engine, Context *context, VkSurfaceKHR _surface){
		connect( VkInstance(*engine), VkPhysicalDevice(*context), VkDevice(*context), _surface );
	}

	void SwapChain::connect(VkInstance _instance, VkPhysicalDevice _gpu, VkDevice _device, VkSurfaceKHR _surface){
		instance = _instance;
		device=_device;
		gpu = _gpu;
		surface = _surface;
		detail = querySwapChainSupport(gpu, surface);
	}

	void SwapChain::create(uint32_t* h, uint32_t* w, bool vsync){
		VkSurfaceFormatKHR _surface_format = chooseSwapSurfaceFormat(detail.formats);
		VkExtent2D _extent = chooseSwapExtent(detail.capabilities, h, w);
		VkPresentModeKHR _present_mode = chooseSwapPresentMode(detail.present_modes);
		uint32_t nr_images = detail.capabilities.minImageCount + 1;

		if(detail.capabilities.maxImageCount > 0 && nr_images > detail.capabilities.maxImageCount)
			nr_images = detail.capabilities.maxImageCount;
		
		VkSwapchainCreateInfoKHR swapchain_CI = infos::swapchainCreateInfo();
		swapchain_CI.surface = surface;
		swapchain_CI.minImageCount = nr_images;
		swapchain_CI.imageFormat = _surface_format.format;
		swapchain_CI.imageColorSpace = _surface_format.colorSpace;
		swapchain_CI.imageExtent = _extent;
		swapchain_CI.imageArrayLayers = 1;
		swapchain_CI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_CI.oldSwapchain=swapchain;
		QueueFamilyIndice indice = Context::findQueueFamilies(gpu, surface);

		uint32_t queue_family_indices[2] = { indice.graphics.value(), indice.present.value() };

		if( indice.present != indice.graphics){
			swapchain_CI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_CI.queueFamilyIndexCount = 2;
			swapchain_CI.pQueueFamilyIndices = queue_family_indices;
		}else{
			swapchain_CI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		swapchain_CI.preTransform = detail.capabilities.currentTransform;
		swapchain_CI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_CI.presentMode = _present_mode;
		swapchain_CI.clipped = VK_TRUE;

		VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &swapchain_CI, nullptr, &swapchain));
		vector<VkImage> images = getSwapchainImages(device, swapchain);
		buffers.resize(images.size());
		for(uint32_t i = 0 ; i < images.size() ; ++i){
			buffers[i].image = images[i];
		}
	
		image_format = _surface_format.format;
		extent = _extent;
		setupImageViews();
	}

	VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& available_formats){
		for(VkSurfaceFormatKHR available_format : available_formats){
			if(available_format.format == VK_FORMAT_R8G8B8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
				return available_format;
			}
		}
		return available_formats[0];
	}

	VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes){
		for(VkPresentModeKHR available_present_mode : available_present_modes){
			if(available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR){
				return available_present_mode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::chooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities, uint32_t *h, uint32_t *w){
		// TODO : should be redesign for user input.
		if(capabilities.currentExtent.width == UINT32_MAX){
			capabilities.currentExtent.width = *w;
			capabilities.currentExtent.height = *h;
		}

		return capabilities.currentExtent;
	}

	void SwapChain::setupImageViews(){
		for(uint32_t i = 0 ; i < buffers.size() ; ++i){
			VkImageViewCreateInfo image_view_CI = infos::imageViewCreateInfo();
			image_view_CI.image = buffers[i].image;
			image_view_CI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_CI.format = image_format;
			image_view_CI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_CI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_CI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_CI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_CI.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			VK_CHECK_RESULT(vkCreateImageView(device, &image_view_CI, nullptr, &buffers[i].view));
		}
	}



	void SwapChain::destroy(){
		assert(device);
		LOG("SwapChain::destroy\n");
		if(swapchain!=VK_NULL_HANDLE){
			LOG("if(swapchain)\n");
			for(auto buffer : buffers){
				vkDestroyImageView(device, buffer.view, nullptr);
			}
		}

		if(surface!=VK_NULL_HANDLE){
			LOG("if(surface)\n");
			vkDestroySwapchainKHR(device, swapchain, nullptr);
			#ifndef GLFW_INCLUDE_VULKAN
			vkDestroySurfaceKHR(instance, surface, nullptr);
			#endif
		}
		swapchain = VK_NULL_HANDLE;
		#ifndef GLFW_INCLUDE_VULKAN
		surface = VK_NULL_HANDLE;
		#endif
	}

	void SwapChain::acquiredNextImage(VkSemaphore present_complete_semaphore, uint32_t *image_index){
		VK_CHECK_RESULT(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, present_complete_semaphore, 
									 (VkFence)nullptr, image_index));
	}

	void SwapChain::queuePresent(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore){
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain;
		present_info.pImageIndices = &image_index;
		present_info.pNext = NULL;

		if(wait_semaphore){
			present_info.pWaitSemaphores = &wait_semaphore;
			present_info.waitSemaphoreCount = 1;
		}

		VK_CHECK_RESULT(vkQueuePresentKHR(queue, &present_info));
	}
}


#endif