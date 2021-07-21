#ifndef __VK_CONTEXT_H__
#define __VK_CONTEXT_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include "vk_engine.h"
#include "vk_infos.h"
#include "vk_utils.h"
#include "vk_queue_family.h"

using namespace std;

namespace VKEngine{
	class Context{
		private :
		const VkInstance instance;
		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkQueueFlags queue_family_flags;
		vector<VkPhysicalDeviceFeatures> device_features; 
		
		private :
		VkPhysicalDevice selectGPU(const VkInstance instance, const uint32_t gpu_id);
		void setupDevice();
		public :
		explicit Context(const VkInstance instance, const uint32_t gpu_id, VkQueueFlags _queue_family_flags);
		QueueFamilyIndice findQueueFamilies();
		QueueFamilyIndice findQueueFamilies(VkPhysicalDevice _gpu);
		operator VkPhysicalDevice() const{
			return gpu;
		}
		operator VkDevice() const{
			return device;
		}
	};
}

#endif