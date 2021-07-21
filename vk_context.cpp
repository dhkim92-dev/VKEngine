#ifndef __VK_CONTEXT_CPP__
#define __VK_CONTEXT_CPP__

#include "vk_context.h"

using namespace std;

namespace VKEngine{
	Context::Context( const VkInstance _instance, const uint32_t gpu_id, VkQueueFlags _queue_family_flags) : instance(_instance){
		queue_family_flags = _queue_family_flags;
		gpu = selectGPU(instance, gpu_id);
	}

	VkPhysicalDevice Context::selectGPU(const VkInstance instance, const uint32_t gpu_id){
		vector<VkPhysicalDevice> gpus = enumerateGPU(instance);

		return gpus[gpu_id];
	}

	QueueFamilyIndice Context::findQueueFamilies(){
		return findQueueFamilies(gpu);
	}

	QueueFamilyIndice Context::findQueueFamilies(VkPhysicalDevice _gpu){
		QueueFamilyIndice indice;
		vector<VkQueueFamilyProperties> properties = enumerateQueueFamilyProperties(gpu);
		if(properties.size() <= 0){
			throw runtime_error("No Queue Family Properties support on this GPU");
		}

		int i = 0;
		for(const VkQueueFamilyProperties property : properties){
			if(property.queueFlags & VK_QUEUE_GRAPHICS_BIT){
				indice.graphics = i;
			}
			if(property.queueFlags & VK_QUEUE_TRANSFER_BIT){
				indice.transfer = i;
			}
			if(property.queueFlags & VK_QUEUE_COMPUTE_BIT){
				indice.compute = i;
			}

			
		}
	}
}

#endif