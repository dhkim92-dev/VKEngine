#ifndef __VK_SWAPCHAIN_H__
#define __VK_SWAPCHAIN_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "vk_engine.h"
#include "vk_context.h"
#include "vk_queue.h"
#include "vk_utils.h"
#include "vk_infos.h"

using namespace std;

namespace VKEngine{

class Swapchain{
	private:
	Context *ctx;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkSurfaceCapabilitiesKHR capabilities;
	VkPresentModeKHR present_mode;
	VkSurfaceFormatKHR format;
	VkExtent2D extent;
	vector<VkImageView> views;
	VkSharingMode image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
	uint32_t present_queue_index=UINT32_MAX;
	bool use_extenal=false;
	bool vsync = false;
	uint32_t image_count;

	private:
	VkBool32 getSurfaceSupport(uint32_t queue_index);
	void setupPresentQueue();
	void setupSurfaceCapabilities();
	void setupPresentMode();
	void setupExtent(uint32_t *width, uint32_t* height);
	vector<VkSurfaceFormatKHR> getSurfaceFormats();
	void selectFormat(vector<VkSurfaceFormatKHR>& _formats);
	VkCompositeAlphaFlagBitsKHR getCompositeAlpha();
	VkResult setupImageViews();

	public:
	explicit Swapchain(Context *ctx, VkSurfaceKHR surface);
	~Swapchain();
	void init();
	VkResult create(uint32_t *width, uint32_t *height);
	//getter
	VkSwapchainKHR* getSwapchain();
	vector<VkImageView> getImageViews();
	VkSurfaceFormatKHR getFormat();
	uint32_t getImageCount();

	//setter
	void setVsync(bool value);
	void setImageSharingMode(VkSharingMode mode);
	void setSwapchain(VkSwapchainKHR _swapchain);
};


class Presenter
{
	public:
	static VkResult acquire(Context *ctx, Swapchain *swapchain, uint32_t *image_index, VkSemaphore *present_complete_smp=nullptr)
	{
		return vkAcquireNextImageKHR(ctx->getDevice(), *swapchain->getSwapchain(), UINT64_MAX, *present_complete_smp,(VkFence)nullptr, image_index );
	}

	static VkResult present(CommandQueue *queue,Swapchain* swapchain, uint32_t *image_index, VkSemaphore *wait_smp=nullptr)
	{
		return queue->present(swapchain->getSwapchain(), 1, image_index, wait_smp);
	}
};


}

#endif