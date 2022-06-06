#ifndef __VK_PHYSICALDEVICE_H__
#define __VK_PHYSICALDEVICE_H__

#include <stdexcept>
#include <vulkan/vulkan.h>
#include "vk_engine.h"
#include "vk_queue_family.h"
#include "vk_utils.h"

namespace VKEngine{

class PhysicalDevice{
//params
private:
	Engine *engine;
	VkPhysicalDevice device=VK_NULL_HANDLE;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties mem_properties;
// methods
private:
	void prepareDeviceProperties();
public: 
	explicit PhysicalDevice(Engine* engine);
	void init();
	void useGPU(int id);
	VkBool32 findQueueFamilyIndice(QueueFamilyIndice *pindice,VkQueueFlags flags);
	uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags flags, VkBool32* found=nullptr);

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