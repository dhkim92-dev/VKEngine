#ifndef __VK_CONTEXT_CPP__
#define __VK_CONTEXT_CPP__

#include "vk_context.h"

using namespace std;

#ifdef VKENGINE_GRAPHICS_MODE
bool graphics_mode=true;
#else
bool graphics_mode=false;
#endif

namespace VKEngine{
	Context::Context(){

	}
	Context::Context(const VkInstance _instance,
		const uint32_t gpu_id, 
		const VkQueueFlags request_queues,
		const vector<const char *>_extension_names,
		const vector<const char *>_validation_names
	){
		create(_instance, gpu_id, request_queues, _extension_names, _validation_names);
	}

	Context::~Context(){
		destroy();
	}

	void Context::create(VkInstance _instance, uint32_t gpu_id, VkQueueFlags request_queues, vector<const char *> device_extensions, vector<const char *> validation_extensions){
		this->instance = _instance;
		selectGPU(gpu_id);
		setupQueueFamilyIndices();
		setupDevice(request_queues, device_extensions, validation_extensions);
		setupMemoryProperties();
	}


	void Context::setupMemoryProperties(){
		vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);
	}

	uint32_t Context::getMemoryType(uint32_t type, VkMemoryPropertyFlags property, VkBool32 *found){
		for(uint32_t i = 0 ; memory_properties.memoryTypeCount ; ++i){
			if(type&1){
				if((memory_properties.memoryTypes[i].propertyFlags&property)==property){
					if(found)
						*found = VK_TRUE;
					return i;
				}
			}
			type>>=1;
		}

		if(found){
			*found = VK_FALSE;
			return 0;
		}else{
			std::runtime_error("can not found matched memory type");
		}
	}

	void Context::destroy(){
		if(graphics_pool) vkDestroyCommandPool(device, graphics_pool, nullptr);
		graphics_pool = VK_NULL_HANDLE;
		if(compute_pool) vkDestroyCommandPool(device, compute_pool, nullptr);
		compute_pool = VK_NULL_HANDLE;
		//if(surface) vkDestroySurfaceKHR(instance, surface, nullptr);
		if(device) vkDestroyDevice(device, nullptr);
		device = VK_NULL_HANDLE;
	}

	void Context::selectGPU(const uint32_t gpu_id){
		vector<VkPhysicalDevice> gpus(enumerateGPU(instance));
		gpu = gpus[gpu_id];
	}

	void Context::setupDevice(VkQueueFlags request_queue, vector<const char*> device_exts, vector<const char*> valid_exts){
		LOG("setupDevice!\n");
		LOG("Selected GPU : %p!\n", gpu);
		/*
		for(const char * name : device_exts){
			LOG("Request Extension : %s\n", name);
		}
		for(const char * name : valid_exts){
			LOG("Requested Validation : %s\n", name);
		}
		*/
		LOG("setupDevice!\n");
		queue_family_indices = findQueueFamilies(gpu);
		vector<VkDeviceQueueCreateInfo> device_queue_CI;
		set<uint32_t> unique_queue_families;
		VkDeviceCreateInfo device_CI = infos::deviceCreateInfo();
		float queue_priorities = 1.0f;
		LOG("unique queue family size : %d\n", unique_queue_families.size());

		if(request_queue & VK_QUEUE_GRAPHICS_BIT){
			if(!queue_family_indices.graphics.has_value()){
				std::runtime_error("This device not support Graphics Queue Family.");
			}
			unique_queue_families.insert( queue_family_indices.graphics.value() );
		}

		if(request_queue & VK_QUEUE_COMPUTE_BIT){
			if(!queue_family_indices.compute.has_value()){
				if(!queue_family_indices.graphics.has_value()){
					std::runtime_error("This device has no suitable compute queue");
				}
				unique_queue_families.insert(queue_family_indices.graphics.value());
			}else{
				unique_queue_families.insert(queue_family_indices.compute.value());
			}
		}

		if(request_queue & VK_QUEUE_TRANSFER_BIT){
			if(!queue_family_indices.transfer.has_value()){
				if(!queue_family_indices.graphics.has_value()){
					std::runtime_error("This device has no suitable transfer queue");
				}
				unique_queue_families.insert(queue_family_indices.graphics.value());
			}else{
				unique_queue_families.insert(queue_family_indices.transfer.value());
			}
		}

		if(unique_queue_families.size() == 0){
			std::runtime_error("This Device has no Suitable Queue Family. Maybe not support Vulkan API.");
		}

		for(uint32_t queue_family : unique_queue_families){
			VkDeviceQueueCreateInfo queue_CI = infos::deviceQueueCreateInfo();
			queue_CI.queueCount = 1;
			queue_CI.queueFamilyIndex = queue_family;
			queue_CI.pQueuePriorities = &queue_priorities;
			device_queue_CI.push_back(queue_CI);
		}

		device_features ={};
		device_CI.pEnabledFeatures = &device_features;
		device_CI.enabledLayerCount = static_cast<uint32_t>(valid_exts.size());
		device_CI.enabledExtensionCount = static_cast<uint32_t>(device_exts.size());
		device_CI.ppEnabledLayerNames = valid_exts.data();
		device_CI.ppEnabledExtensionNames = device_exts.data();
		device_CI.queueCreateInfoCount = static_cast<uint32_t>(device_queue_CI.size());
		device_CI.pQueueCreateInfos = device_queue_CI.data();
		VK_CHECK_RESULT( vkCreateDevice(gpu, &device_CI, nullptr, &device) );
	}

	VkCommandPool Context::createCommandPool(uint32_t queue_index, VkCommandPoolCreateFlags flags){
		VkCommandPool pool;
		VkCommandPoolCreateInfo command_pool_CI = infos::commandPoolCreateInfo(queue_index,flags);
		VK_CHECK_RESULT( vkCreateCommandPool(device, &command_pool_CI, nullptr, &pool) );
		return pool;
	}

	VkCommandPool Context::getCommandPool(VkQueueFlagBits type){
		VkCommandPool ret = VK_NULL_HANDLE;
		switch(type){
			case VK_QUEUE_GRAPHICS_BIT : 
				ret = graphics_pool;
				break;
			case VK_QUEUE_COMPUTE_BIT :
				ret = compute_pool;
				break;
			default :
				ret = graphics_pool;
		}
		assert(ret!=VK_NULL_HANDLE);
		return ret;
	}

	void Context::setupQueueFamilyIndices(){
		queue_family_indices = findQueueFamilies(gpu);
	}

	QueueFamilyIndice Context::findQueueFamilies(VkPhysicalDevice _gpu){
		QueueFamilyIndice indice;
		vector<VkQueueFamilyProperties> properties(enumerateQueueFamilyProperties(_gpu));
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

			if(indice.isSupport(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)){
				break;
			}

			++i;
		}

		return indice;
	}
}

#endif