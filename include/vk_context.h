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
		Engine *engine = nullptr;
		VkInstance instance;
		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;

		private :
		void setupMemoryProperties();
		public :
		VkPhysicalDeviceFeatures device_features;
		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceMemoryProperties memory_properties;
		QueueFamilyIndice queue_family_indices;
		Context();
		Context(Engine *_engine,
			const uint32_t gpu_id,  
			const VkQueueFlags request_queues,
			VkSurfaceKHR surface
		);

		void destroy();
		void setupSurface(VkSurfaceKHR _surface);		
		void setupQueueFamilyIndices();

		~Context();

		void create(Engine *_engine, uint32_t gpu_id, VkQueueFlags request_queues, VkSurfaceKHR surface);
		void selectGPU(const uint32_t gpu_id);
		void setupDevice(VkQueueFlags request_queue, vector<const char*>device_exts, vector<const char*> valid_exts);
		QueueFamilyIndice findQueueFamilies(VkPhysicalDevice _gpu);
		//VkCommandPool createCommandPool(uint32_t queue_index, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandPool createCommandPool(VkQueueFlagBits type, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags property, VkBool32 *found=nullptr);
		uint32_t getQueueFamilyIndex(VkQueueFlagBits queue_family);
		void setupPresent(VkSurfaceKHR surface);
		/*
		operator VkSurfaceKHR() const {
			return surface;
		}
		*/

		operator VkPhysicalDevice() const{
			return gpu;
		}

		operator VkDevice() const{
			return device;
		}
	};
}

#endif
