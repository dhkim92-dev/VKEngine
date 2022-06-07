#ifndef __VK_PHYSICALDEVICE_H__
#define __VK_PHYSICALDEVICE_H__

#include <stdexcept>
#include <vulkan/vulkan.h>
#include <algorithm>
#include <vector>
#include <set>
#include <cstring>
#include "vk_engine.h"
#include "vk_queue_family.h"
#include "vk_utils.h"
using namespace std;

namespace VKEngine{

class PhysicalDevice{
//params
private:
	Engine *engine;
	VkPhysicalDevice device=VK_NULL_HANDLE;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties mem_properties;
	vector<const char *> support_device_extensions;
// methods
private:
	void prepareDeviceProperties();
	void prepareMemoryProperties();
	void prepareDeviceFeatures();
	void prepareDeviceExtensions();
public: 
	explicit PhysicalDevice(Engine* engine);
	void init();
	void useGPU(int id);
	VkBool32 findQueueFamilyIndice(QueueFamilyIndice *pindice,VkQueueFlags flags);
	uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags flags, VkBool32* found=nullptr);
	bool isSupportDeviceExtension(const char *ext_name);
	//setter
	void setPhysicalDevice(VkPhysicalDevice pdevice);
	void setQueueFlags(VkQueueFlags queue_flags);
	//getter
	VkPhysicalDevice getPhysicalDevice();
	VkPhysicalDeviceFeatures getDeviceFeatures();
	VkPhysicalDeviceProperties getDeviceProperties();
	VkPhysicalDeviceMemoryProperties getMemoryProperties();
	Engine* getEngine();

};

};
#endif