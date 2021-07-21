#ifndef __VK_SWAPCHAIN_H__
#define __VK_SWAPCHAIN_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include "vk_context.h"
#include "vk_utils.h"
#include "vk_infos.h"

using namespace std;

namespace VKEngine{
	struct SwapChainBuffer{
		VkImage image;
		VkImageView view;
		VkFramebuffer framebuffer;
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
		VkSurfaceFormatKHR surface_format;
		VkPresentModeKHR present_mode;
		VkExtent2D extent;
		vector<SwapChainBuffer> buffers;

		public:
		void connect(VkInstance _instance, VkPhysicalDevice _gpu, VkDevice _device);
		void connect(VkInstance _instance, VkPhysicalDevice _gpu, VkDevice _device, VkSurfaceKHR _surface);
		void create(uint32_t height, uint32_t width, bool vsync=false);
		void acquiredNextImage(VkSemaphore present_complete_semaphore, uint32_t image_index);
		void queuePresent(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore);
		void destroy();
		void recreate();

		private:
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& available_formats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void setupSwapchainBuffers();
		void setupImages();
		void setupImageViews();
		void setupFramebuffers();
	};
}

#endif