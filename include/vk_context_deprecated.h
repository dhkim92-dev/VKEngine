#ifndef __VK_CONTEXT_H__
#define __VK_CONTEXT_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include "vk_engine.h"
#include "vk_physicaldevice.h"
#include "vk_infos.h"
#include "vk_utils.h"
#include "vk_queue_family.h"

using namespace std;

namespace VKEngine{
	class Context{
		private :
		Engine *engine = nullptr;
		PhysicalDevice *pdevice;
		VkInstance instance;
		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkPhysicalDeviceFeatures device_features;
		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceMemoryProperties memory_properties;
		QueueFamilyIndice queue_family_indices;

		private :
		void setupMemoryProperties();
		public :
		Context();
		Context(PhysicalDevice *pdevice);
		Context(Engine *_engine,
			const uint32_t gpu_id,  
			const VkQueueFlags request_queues);
		~Context();
		void init();
		void destroy();
		void setupQueueFamilyIndices();
		void create(Engine *_engine, uint32_t gpu_id, VkQueueFlags request_queues);
		void setupPhysicalDevice(const uint32_t gpu_id);
		void setupDevice(VkQueueFlags request_queue, vector<const char*>device_exts, vector<const char*> valid_exts);
		QueueFamilyIndice findQueueFamilies(VkPhysicalDevice _gpu);
		VkCommandPool createCommandPool(VkQueueFlagBits type, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkResult createSemaphore(VkSemaphore *semaphore);
		VkResult createEvent(VkEvent *event);
		VkResult createFence(VkFence *fence);

		void destroySemaphore(VkSemaphore *semaphore);
		void destroyEvent(VkEvent *event);
		void destroyFence(VkFence *fence);

		// present 관련 함수
		void useSurface(VkSurfaceKHR _surface);		

		//setter
		void setEngine(Engine* _engine);
		void setPhysicalDevice(VkPhysicalDevice device);
		void setDevice(VkDevice device);

		//getter
		uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags property, VkBool32 *found=nullptr);
		uint32_t getQueueFamilyIndex(VkQueueFlagBits queue_family);
		VkPhysicalDevice getPhysicalDevice() const {
			return this->gpu;
		}
		VkDevice getDevice(){
			return this->device;
		}
	};
}

#endif
