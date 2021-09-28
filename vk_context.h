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
		VkInstance instance;
		//const VkSurfaceKHR surface;
		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		//const vector<const char *> validation_names;
		//const vector<const char *> extension_names;
		//VkQueue graphics_queue, compute_queue, transfer_queue, present_queue;
		VkPhysicalDeviceFeatures device_features;
		VkPhysicalDeviceMemoryProperties memory_properties;

		VkCommandPool graphics_pool = VK_NULL_HANDLE; // default command pool for graphics.
		VkCommandPool compute_pool = VK_NULL_HANDLE; // default command pool for compute.
		VkCommandPool transfer_pool = VK_NULL_HANDLE; // default command pool for transfer;

		private :
		void destroy();
		void setupMemoryProperties();
		public :
		QueueFamilyIndice queue_family_indices;
		Context(const VkInstance instance,
			const uint32_t gpu_id,  
			const VkQueueFlags request_queues,
			const vector<const char *> _extension_names,
			const vector<const char *> _validation_names);
		Context();

		void setupPresentFamily(VkSurfaceKHR _surface);		
		void setupQueueFamilyIndices();

		~Context();

		void create(VkInstance instance, uint32_t gpu_id, VkQueueFlags request_queues, vector<const char*> device_extensions, vector<const char*> validation_extensions);
		void selectGPU(const uint32_t gpu_id);
		void setupDevice(VkQueueFlags request_queue, vector<const char*>device_exts, vector<const char*> valid_exts);
		QueueFamilyIndice findQueueFamilies(VkPhysicalDevice _gpu);
		VkCommandPool createCommandPool(uint32_t queue_index, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandPool getCommandPool(VkQueueFlagBits type);
		uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags property, VkBool32 *found=nullptr);
		uint32_t getQueueFamilyIndex(VkQueueFlagBits queue_family);
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