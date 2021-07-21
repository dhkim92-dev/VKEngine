#ifndef __VK_CONTEXT_H__
#define __VK_CONTEXT_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include "vk_engine.h"
#include "vk_infos.h"
#include "vk_utils.h"
#include "vk_queue_family.h"
#include "vk_queue.h"

using namespace std;

namespace VKEngine{
	class Context{
		private :
		const VkInstance instance;
		const VkSurfaceKHR surface;
		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		const VkQueueFlags queue_family_flags;
		const vector<const char *> validation_names;
		const vector<const char *> extension_names;
		VkQueue graphics_queue, compute_queue, transfer_queue, present_queue;
		VkPhysicalDeviceFeatures device_features;
		
		private :
		void setupDevice();
		void destroy();
		bool isSuitableGPU(VkPhysicalDevice _gpu);
		public :
		explicit Context(const VkInstance instance,
			const uint32_t gpu_id,  
			const VkSurfaceKHR _surface, 
			const VkQueueFlags _queue_family_flags,
			const vector<const char *> _extension_names,
			const vector<const char *> _validation_names);
		~Context();
		void selectGPU(const uint32_t gpu_id);
		QueueFamilyIndice findQueueFamilies();
		static QueueFamilyIndice findQueueFamilies(VkPhysicalDevice _gpu, VkSurfaceKHR _surface);
		CommandQueue createCommandQueue( VkQueueFlagBits type );
		operator VkSurfaceKHR() const {
			return surface;
		}

		operator VkPhysicalDevice() const{
			return gpu;
		}

		operator VkDevice() const{
			return device;
		}
	};
}

#endif