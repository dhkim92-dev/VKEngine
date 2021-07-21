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

namespace VKEngine{
	vector<VkPhysicalDevice> enumerateGPU(VkInstance instance);
	vector<VkLayerProperties> enumerateValidations();
	vector<VkQueueFamilyProperties> enumerateQueueFamilyProperties(VkPhysicalDevice gpu);
	//vector<VkPhysicalDeviceProperties> enumerateGPUProperties();
	//VkShaderModule loadShader(const string file_path);
};

#endif