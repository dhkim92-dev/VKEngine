#ifndef __VK_UTILS_H__
#define __VK_UTILS_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <cstdio>
#include <cassert>
#include <stdexcept>
#include <chrono>

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

#define PROFILING(FPTR, FNAME) ({ \
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now(); \
		FPTR; \
		std::chrono::duration<double> t = std::chrono::system_clock::now() - start; \
		printf("%s operation time : %.4lf seconds\n",FNAME, t.count()); \
})



namespace VKEngine{
	struct SwapChainSupportDetail{
		VkSurfaceCapabilitiesKHR capabilities;
		vector<VkSurfaceFormatKHR> formats;
		vector<VkPresentModeKHR> present_modes;
	};

	vector<VkPhysicalDevice> enumerateGPU(VkInstance instance);
	vector<VkLayerProperties> enumerateValidations();
	vector<VkQueueFamilyProperties> enumerateQueueFamilyProperties(VkPhysicalDevice gpu);
	vector<VkImage> getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain);
	SwapChainSupportDetail querySwapChainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	uint32_t getMemoryType(VkPhysicalDevice gpu, VkDevice device, uint32_t type, VkMemoryPropertyFlags properties, VkBool32 *mem_found = nullptr );
	VkShaderModule loadShader(const string file_path, VkDevice device);
	VkBool32 getDepthFormat(VkPhysicalDevice gpu, VkFormat *format);
};

#endif