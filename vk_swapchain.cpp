#ifndef __VK_SWAPCHAIN_CPP__
#define __VK_SWAPCHAIN_CPP__

#include "vk_swapchain.h"

using namespace std;

namespace VKEngine{
	void SwapChain::connect(VkInstance _instance, VkPhysicalDevice _gpu, VkDevice _device, VkSurfaceKHR _surface){
		instance = _instance;
		device=_device;
		gpu = _gpu;
		surface = _surface;
		detail = querySwapChainSupport(gpu, surface);
	}

	void SwapChain::create(uint32_t h, uint32_t w, bool vsync){
		present_mode = chooseSwapPresentMode(detail.present_modes);
		uint32_t nr_images = detail.capabilities.minImageCount + 1;
		if(detail.capabilities.maxImageCount > 0 && nr_images > detail.capabilities.maxImageCount){
			nr_images = detail.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchain_CI = infos::swapchainCreateInfo();
		swapchain_CI.surface = surface;
		swapchain_CI.minImageCount = nr_images;
		swapchain_CI.imageFormat = surface_format.format;
		swapchain_CI.imageColorSpace = surface_format.colorSpace;
		swapchain_CI.imageExtent = extent;
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
		swapchain_CI.presentMode = present_mode;
		swapchain_CI.clipped = VK_TRUE;

		VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &swapchain_CI, nullptr, &swapchain));
		surface_format = chooseSwapSurfaceFormat(detail.formats);
		extent = chooseSwapExtent(detail.capabilities);
	}

	void SwapChain::setupSwapchainBuffers(){
		setupImages();
		setupImageViews();
		setupFramebuffers();
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

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
		// TODO : should be redesign for user input.
		if(capabilities.currentExtent.width != UINT32_MAX){
			return {-1, -1};
		}
		return capabilities.currentExtent;
	}
}


#endif