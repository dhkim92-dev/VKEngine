#ifndef __VK_UTILS_H__
#define __VK_UTILS_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdio>
#include <cassert>

using namespace std;

#define LOG(...) printf(__VA_ARGS__)
#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		printf("VK_ERROR! on  %s line : %d \n", __FILE__,__LINE__); \
		assert(res == VK_SUCCESS);																		\
	}																									\
}																						

struct SwapChainSupportDetail{
	VkSurfaceCapabilitiesKHR capabilities;
	vector<VkSurfaceFormatKHR> formats;
	vector<VkPresentModeKHR> present_modes;
};

namespace VKEngine{
	vector<VkPhysicalDevice> enumerateGPU(VkInstance instance);
	vector<VkLayerProperties> enumerateValidations();
	vector<VkQueueFamilyProperties> enumerateQueueFamilyProperties(VkPhysicalDevice gpu);
	vector<VkImage> getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain);
	SwapChainSupportDetail querySwapChainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	
	//VkShaderModule loadShader(const string file_path);
};

#endif