#ifndef __VK_CONTEXT_CPP__
#define __VK_CONTEXT_CPP__

#include "vk_context_deprecated.h"

using namespace std;
namespace VKEngine{
	Context::Context(
		PhysicalDevice *pdevice
	) : pdevice(pdevice) {
	}

	Context::Context(
		Engine *_engine,
		const uint32_t gpu_id, 
		const VkQueueFlags request_queues
	){
		create(_engine, gpu_id, request_queues);
	}

	Context::~Context(){
		LOG("Context::~Context()\n");
		destroy();
		LOG("Context::~Context() end.\n");
	}

	void Context::create(Engine *_engine, uint32_t gpu_id, VkQueueFlags request_queues){
		this->engine= _engine;
		this->instance = _engine->getInstance();
		setupPhysicalDevice(gpu_id);
		setupQueueFamilyIndices();
		setupDevice(request_queues, 
					engine->getDeviceExtensions(), 
					engine->getValidationLayers()
		);
		setupMemoryProperties();
	}

	VkCommandPool Context::createCommandPool(VkQueueFlagBits type, VkCommandPoolCreateFlags flags){
		VkCommandPool pool = VK_NULL_HANDLE;
		uint32_t queue_index = 255;
		switch(type){
			case VK_QUEUE_GRAPHICS_BIT : 
				queue_index = queue_family_indices.graphics.value();
				break;
			case VK_QUEUE_COMPUTE_BIT :
				queue_index = queue_family_indices.compute.value();
				break;
			case VK_QUEUE_TRANSFER_BIT : 
				queue_index = queue_family_indices.transfer.value();
				break;
			default :
				break;
		}
		VkCommandPoolCreateInfo info = infos::commandPoolCreateInfo(queue_index, flags);
		VK_CHECK_RESULT(vkCreateCommandPool(device, &info, nullptr, &pool));
		return pool;
	}

	void Context::setupMemoryProperties(){
		vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);
	}

	uint32_t Context::getMemoryType(uint32_t type, VkMemoryPropertyFlags property, VkBool32 *found){
		for(uint32_t i = 0 ; i <  memory_properties.memoryTypeCount ; ++i){
			if((type&1)==1){
				if((memory_properties.memoryTypes[i].propertyFlags&property)==property){
					if(found)
						*found = VK_TRUE;
					return i;
				}
			}
			type>>=1;
		}

		if(found){
			LOG("found : %d\n", found );
			*found = VK_FALSE;
			return 0;
		}else{
			std::runtime_error("can not found matched memory type");
		}
	}

	void Context::destroy(){
		LOG("Context::destroy()\n");
		if(device) {
			vkDestroyDevice(device, nullptr);
			device = VK_NULL_HANDLE;
		}
		LOG("Context::destroy() end\n");
	}

	void Context::setupPhysicalDevice(const uint32_t gpu_id){
		vector<VkPhysicalDevice> gpus(enumerateGPU(instance));
		LOG("searched vulkan device count : %d\n", gpus.size());
		gpu = gpus[gpu_id];
	}

	void Context::setupDevice(
		VkQueueFlags request_queue, 
		vector<const char*> device_exts, 
		vector<const char*> valid_exts
	){
		// LOG("Selected GPU : %p!\n", gpu);
		queue_family_indices = findQueueFamilies(gpu);
		vector<VkDeviceQueueCreateInfo> device_queue_CI;
		set<uint32_t> unique_queue_families;
		VkDeviceCreateInfo device_CI = infos::deviceCreateInfo();
		float queue_priorities = 1.0f;

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

		if(queue_family_indices.present.has_value()){
			unique_queue_families.insert(queue_family_indices.present.value());
		}

		// LOG("unique queue family size : %d\n", unique_queue_families.size());
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

		vkGetPhysicalDeviceProperties(gpu, &device_properties);
		VK_CHECK_RESULT( vkCreateDevice(gpu, &device_CI, nullptr, &device) );
	}

	void Context::setupQueueFamilyIndices(){
		queue_family_indices = findQueueFamilies(gpu);
	}

	QueueFamilyIndice Context::findQueueFamilies(VkPhysicalDevice _gpu){
		QueueFamilyIndice indice;
		vector<VkQueueFamilyProperties> properties(enumerateQueueFamilyProperties(_gpu));
		cout << "VkQueueFamilyProperties size : "<< properties.size();
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


	
	// 0.1.4 add end
	VkResult Context::createFence(VkFence *fence){
		VkFenceCreateInfo CI = infos::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
		return vkCreateFence(device, &CI, nullptr, fence);
	}

	VkResult Context::createEvent(VkEvent *event){
		VkEventCreateInfo CI = {};
		CI.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
		return vkCreateEvent(device, &CI, nullptr, event);
	}

	VkResult Context::createSemaphore(VkSemaphore *semaphore){
		VkSemaphoreCreateInfo CI = infos::semaphoreCreateInfo();
		return vkCreateSemaphore(device, &CI, nullptr, semaphore);
	}

	void Context::destroyFence(VkFence *fence){
		if(*fence != VK_NULL_HANDLE){
			vkDestroyFence(device, *fence, nullptr);
			*fence = VK_NULL_HANDLE;
		}
	}
	
	void Context::destroyEvent(VkEvent *event){
		if(*event != VK_NULL_HANDLE){
			vkDestroyEvent(device, *event, nullptr);
			*event = VK_NULL_HANDLE;
		}
	}
	
	void Context::destroySemaphore(VkSemaphore *semaphore){
		if(*semaphore != VK_NULL_HANDLE){
			vkDestroySemaphore(device, *semaphore, nullptr);
			*semaphore = VK_NULL_HANDLE;
		}
	}

	// setter
	void Context::setEngine(Engine *_engine){
		this->engine = _engine;
	}

	void Context::setPhysicalDevice(VkPhysicalDevice physical_device){
		this->gpu = physical_device;
	}

	void Context::setDevice(VkDevice _device){
		this->device = _device;
	}
	

	// About Surface
	void Context::useSurface(VkSurfaceKHR surface){
		VkBool32 surface_support = VK_FALSE;
		vector<VkQueueFamilyProperties> properties = enumerateQueueFamilyProperties(gpu);
		for(uint32_t i = 0 ; i < properties.size() ; ++i){
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &surface_support);
			if(surface_support == VK_SUCCESS){
				queue_family_indices.present = i;
			}
		}

		if(surface_support == VK_FALSE){
			std::runtime_error("This GPU has no surface support.");
		}
	}
}

#endif