#ifndef __VK_SWAPCHAIN_H__
#define __VK_SWAPCHAIN_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include "vk_engine.h"
#include "vk_context.h"
#include "vk_utils.h"
#include "vk_infos.h"

using namespace std;

namespace VKEngine{
	struct SwapChainBuffer{
		VkImage image;
		VkImageView view;
	};

	class SwapChain{
		private:
		VkInstance instance;
		VkPhysicalDevice gpu;
		VkDevice device;
		VkSurfaceKHR surface;
		public:
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		SwapChainSupportDetail detail;
		VkFormat image_format;
		VkPresentModeKHR present_mode;
		VkExtent2D extent;
		vector<SwapChainBuffer> buffers;

		public:
		void connect(Engine *engine, Context * context, VkSurfaceKHR _surface);
		void connect(VkInstance _instance, VkPhysicalDevice _gpu, VkDevice _device);
		void connect(VkInstance _instance, VkPhysicalDevice _gpu, VkDevice _device, VkSurfaceKHR _surface);
		void create(uint32_t *_height, uint32_t* _width, bool vsync=false);
		void acquiredNextImage(VkSemaphore present_complete_semaphore, 
							   uint32_t *image_index);
		void queuePresent(VkQueue queue, 
						  uint32_t image_index, 
						  VkSemaphore wait_semaphore = VK_NULL_HANDLE);
		void destroy();

		private:
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& available_formats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
		VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities, uint32_t *h , uint32_t *w);
		void setupImageViews();
	};
}

#endif