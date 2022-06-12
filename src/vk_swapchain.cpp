#ifndef __VK_SWAPCHAIN_CPP__
#define __VK_SWAPCHAIN_CPP__

#include "vk_swapchain.h"

using namespace std;

namespace VKEngine{


Swapchain::Swapchain(Context *ctx, VkSurfaceKHR surface) : ctx(ctx), surface(surface){
	image_count=0;
}

Swapchain::~Swapchain()
{
	if(views.size() != 0){
		for(int i = 0 ; i < views.size() ; i++){
			vkDestroyImageView(ctx->getDevice(), views[i], nullptr);
		}
		views.clear();
	}

	if(swapchain != VK_NULL_HANDLE){
		vkDestroySwapchainKHR(ctx->getDevice(), swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
}

void Swapchain::setSwapchain(VkSwapchainKHR _swapchain)
{
	if(swapchain == VK_NULL_HANDLE){
		swapchain=_swapchain;
		use_extenal = true;
	}
}

void Swapchain::setVsync(bool value){vsync = value;}
void Swapchain::setImageSharingMode(VkSharingMode mode){image_sharing_mode = mode;}
vector<VkImageView> Swapchain::getImageViews(){return views;}
VkSurfaceFormatKHR Swapchain::getFormat(){return format;}
uint32_t Swapchain::getImageCount(){return image_count;}
VkSwapchainKHR* Swapchain::getSwapchain(){return &swapchain;}

void Swapchain::init()
{
	setupPresentQueue();
	vector<VkSurfaceFormatKHR> formats = getSurfaceFormats();
	selectFormat(formats);
}

VkResult Swapchain::create(uint32_t *width, uint32_t *height)
{
	VkSwapchainKHR old_swapchain = swapchain;
	setupSurfaceCapabilities();
	setupExtent(width, height);
	setupPresentMode();
	VkCompositeAlphaFlagBitsKHR composite = getCompositeAlpha();
	QueueFamilyIndice indice = ctx->getQueueFamily();
	uint32_t queue_family_indices[2] = {indice.graphics.value(), indice.present.value()};
	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.imageSharingMode = image_sharing_mode;
	info.imageExtent = extent;
	info.imageFormat = format.format;
	info.imageColorSpace = format.colorSpace;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	info.minImageCount = image_count;
	info.imageArrayLayers=1;
	info.presentMode = present_mode;
	info.oldSwapchain = old_swapchain;
	info.queueFamilyIndexCount = (image_sharing_mode == VK_SHARING_MODE_CONCURRENT) ? 2 : 0;
	info.pQueueFamilyIndices = (image_sharing_mode == VK_SHARING_MODE_EXCLUSIVE) ? nullptr : queue_family_indices;
	info.compositeAlpha = composite;
	info.clipped = VK_TRUE;

	if(capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR){
		info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}else{
		info.preTransform = capabilities.currentTransform;
	}

	VkResult res = vkCreateSwapchainKHR(ctx->getDevice(), &info, nullptr, &swapchain);
	// LOG("swapchain create result : %d\n", res);
	if(res != VK_SUCCESS){
		LOG("fail to create swapchain.\n");
		return res;
	}

	res = setupImageViews();
	return res;
}

void Swapchain::setupExtent(uint32_t *width, uint32_t* height)
{
	if(capabilities.currentExtent.width == (uint32_t)-1){
		extent.width = *width;
		extent.height = *height;
	}else{
		extent.width = capabilities.currentExtent.width;
		extent.height = capabilities.currentExtent.height;
		*height = extent.height;
		*width = extent.width;
	}

	image_count = capabilities.minImageCount+1;
	image_count = (image_count > capabilities.maxImageCount) ? capabilities.maxImageCount : image_count;
}

void Swapchain::setupPresentQueue()
{
	QueueFamilyIndice indice = ctx->getQueueFamily();
	uint32_t count = 0;
	present_queue_index = UINT32_MAX;

	if(image_sharing_mode == VK_SHARING_MODE_EXCLUSIVE){
		if(!indice.graphics.has_value()){
			LOG("Swapchain Image sharing mode exclusive but no Graphics queue support.\n");
			exit(EXIT_FAILURE);
		}
		present_queue_index = indice.graphics.value();
	}else{
		vector<VkQueueFamilyProperties> props = enumerateQueueFamilyProperties(ctx->getPhysicalDevice()->getPhysicalDevice());

		for(uint32_t i = 0 ; i < props.size() ; i++){
			if(i != indice.graphics.value()){
				VkBool32 ret = getSurfaceSupport(i);
				if(ret == VK_TRUE){
					present_queue_index = i;
					break;
				}
			}
		}

		if(present_queue_index == UINT32_MAX){
			LOG("Swapchain image sharing mode force to set EXCLUSIVE mode.\n");
			image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
			if(!indice.graphics.has_value()){
				LOG("Swapchain, No graphics queue supported\n");
				exit(EXIT_FAILURE);
			}
			present_queue_index =  indice.graphics.value();
		}
	}
	ctx->setPresentQueueIndex(present_queue_index);
}

void Swapchain::setupPresentMode()
{
	uint32_t present_mode_count = 0 ;
	vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->getPhysicalDevice()->getPhysicalDevice(), surface, &present_mode_count, nullptr);
	assert(present_mode_count>0);
	vector<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->getPhysicalDevice()->getPhysicalDevice(), surface, &present_mode_count, present_modes.data());

	present_mode = VK_PRESENT_MODE_FIFO_KHR;

	if(!vsync){
		for(int i = 0 ; i < present_mode_count ; i++){
			if(present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
				present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}

			if(present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR){
				present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}
}

VkBool32 Swapchain::getSurfaceSupport(uint32_t queue_index)
{
	VkPhysicalDevice gpu = ctx->getPhysicalDevice()->getPhysicalDevice();
	VkBool32 result = VK_FALSE;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queue_index, surface, &result));
	return result;
}

void Swapchain::setupSurfaceCapabilities()
{
	VkPhysicalDevice gpu = ctx->getPhysicalDevice()->getPhysicalDevice();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities);
}

vector<VkSurfaceFormatKHR> Swapchain::getSurfaceFormats()
{
	vector<VkSurfaceFormatKHR> formats;
	uint32_t count = 0;
	VkPhysicalDevice gpu = ctx->getPhysicalDevice()->getPhysicalDevice();
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &count, nullptr);

	if(count == 0){
		LOG("No Physical Device Surface Format.\n");
		exit(EXIT_FAILURE);
	}
	formats.resize(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &count, formats.data());

	return formats;
}

VkCompositeAlphaFlagBitsKHR Swapchain::getCompositeAlpha()
{
	VkCompositeAlphaFlagBitsKHR flags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
	};

	for(auto flag : flags){
		if(capabilities.supportedCompositeAlpha & flag){
			return flag;
		}
	}

	return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

void Swapchain::selectFormat(vector<VkSurfaceFormatKHR>& _formats)
{
	uint32_t count = _formats.size();

	if( (count == 1) && _formats[0].format == VK_FORMAT_UNDEFINED)
	{
		format.format = VK_FORMAT_B8G8R8A8_SNORM;
		format.colorSpace = _formats[0].colorSpace;
	}else{
		bool found = false;

		for(auto f : _formats)
		{
			if(f.format == VK_FORMAT_B8G8R8A8_SNORM)
			{
				format.format = f.format;
				format.colorSpace = f.colorSpace;
				found = true;
				break;
			}
		}

		if(!found){
			format.format = _formats[0].format;
			format.colorSpace = _formats[0].colorSpace;
		}
	}
}

VkResult Swapchain::setupImageViews()
{
	views.resize(image_count);
	vector<VkImage> swapchain_images(image_count);
	vkGetSwapchainImagesKHR(ctx->getDevice(), swapchain, &image_count, swapchain_images.data());
	for(int i = 0 ; i < views.size() ; i++){
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.image = swapchain_images[i];
		info.format = format.format;
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseArrayLayer=0;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.flags = 0;

		VkResult result = vkCreateImageView(ctx->getDevice(), &info, nullptr, &views[i]);
		if(result != VK_SUCCESS){
			return result;
		}
	}
	return VK_SUCCESS;
}

}


#endif