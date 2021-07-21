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
	Context::Context(const VkInstance _instance,
		const uint32_t gpu_id, 
		const VkSurfaceKHR _surface, 
		const VkQueueFlags _queue_family_flags,
		const vector<const char *>_extension_names,
		const vector<const char *>_validation_names
		) : instance(_instance), surface(_surface), validation_names(_validation_names),extension_names(_extension_names), queue_family_flags(_queue_family_flags)
		{
		
		selectGPU(gpu_id);
		LOG("GPU selected\n");
		if(gpu == VK_NULL_HANDLE){
			throw runtime_error("Failed to set GPU. maybe your computer has no suitable GPU for Vulkan API");
		}
		LOG("logical Device will set\n");
		setupDevice();
	}

	Context::~Context(){
		destroy();
	}

	void Context::destroy(){
		vkDestroyDevice(device, nullptr);
	}

	void Context::selectGPU(const uint32_t gpu_id){
		vector<VkPhysicalDevice> gpus(enumerateGPU(instance));
		VkPhysicalDevice selected_gpu = VK_NULL_HANDLE;

		LOG("searched GPU count : %d\n", gpus.size() );

		bool is_suitable = false;
		int i = 0 ;
		for(VkPhysicalDevice _gpu : gpus){
			LOG("_gpu : %p \n",_gpu);
			is_suitable = isSuitableGPU(_gpu);
			LOG("GPU : %d suitable : %d\n", i, is_suitable);
			if(is_suitable){
				selected_gpu = _gpu;
				break;
			}
			++i;
		}
		
		
		gpu = (gpu == VK_NULL_HANDLE) ? selected_gpu : VK_NULL_HANDLE;
	}

	void Context::setupDevice(){
		LOG("setupDevice!\n");
		LOG("Selected GPU : %p!\n", gpu);
		for(const char * name : extension_names){
			LOG("Request Extension : %s\n", name);
		}
		for(const char * name : validation_names){
			LOG("Requested Validation : %s\n", name);
		}
		LOG("setupDevice!\n");
		QueueFamilyIndice indice = findQueueFamilies();
		vector<VkDeviceQueueCreateInfo> device_queue_CI;
		set<uint32_t> unique_queue_families = { indice.graphics.value(), indice.compute.value(), indice.transfer.value(), indice.present.value() };
		LOG("unique queue family size : %d\n", unique_queue_families.size());
		VkDeviceCreateInfo device_CI = infos::deviceCreateInfo();
		float queue_priorities = 1.0f;
		
		for(uint32_t queue_family : unique_queue_families){
			VkDeviceQueueCreateInfo queue_CI = infos::deviceQueueCreateInfo();
			queue_CI.queueCount = 1;
			queue_CI.queueFamilyIndex = queue_family;
			queue_CI.pQueuePriorities = &queue_priorities;
			device_queue_CI.push_back(queue_CI);
		}

		device_features ={};
		device_CI.pEnabledFeatures = &device_features;
		device_CI.enabledLayerCount = static_cast<uint32_t>(validation_names.size());
		device_CI.enabledExtensionCount = static_cast<uint32_t>(extension_names.size());
		device_CI.ppEnabledLayerNames = validation_names.data();
		device_CI.ppEnabledExtensionNames = extension_names.data();
		device_CI.queueCreateInfoCount = static_cast<uint32_t>(device_queue_CI.size());
		device_CI.pQueueCreateInfos = device_queue_CI.data();

		VK_CHECK_RESULT( vkCreateDevice(gpu, &device_CI, nullptr, &device) );
	}

	CommandQueue Context::createCommandQueue(VkQueueFlagBits type){
		uint32_t index;
		QueueFamilyIndice indice = findQueueFamilies();
		VkQueue queue;
		VkCommandPool pool;
		VkCommandPoolCreateInfo pool_CI;
		
		switch(type){
			case VK_QUEUE_GRAPHICS_BIT :
				index = indice.graphics.value();
				break;
			case VK_QUEUE_TRANSFER_BIT :
				index = indice.transfer.value();
				break;
			case VK_QUEUE_COMPUTE_BIT :
				index = indice.compute.value(); 
				break;
			default :
				index = -1;
		}

		if(index < 0){
			throw runtime_error("no suitable queue family to make command_queue\n");
		}

		vkGetDeviceQueue(device, index, 0, &queue);
		pool_CI = infos::commandPoolCreateInfo(index, 0);
		VK_CHECK_RESULT(vkCreateCommandPool(device, &pool_CI, nullptr, &pool));

		CommandQueue command_queue(device, queue, type, pool);
		return command_queue;
	}

	bool Context::isSuitableGPU(VkPhysicalDevice _gpu){
		bool result = false;
		LOG("isSuitableGPU for %p \n");
		QueueFamilyIndice indice = findQueueFamilies(_gpu, surface);
		LOG("indice : %d %d %d\n", indice.graphics.value(), indice.transfer.value(), indice.compute.value());
		result = indice.isSupport(graphics_mode);
		LOG("is Suitable result : %d\n", result);

		return result;
	}

	QueueFamilyIndice Context::findQueueFamilies(){
		return findQueueFamilies(gpu, surface);
	}

	QueueFamilyIndice Context::findQueueFamilies(VkPhysicalDevice _gpu, VkSurfaceKHR _surface){
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
			}else{
				indice.transfer = indice.graphics.value();
			}

			if(property.queueFlags & VK_QUEUE_COMPUTE_BIT){
				indice.compute = i;
			}else{
				indice.compute = indice.graphics.value();
			}

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(_gpu, i, _surface, &present_support);

			if(present_support){
				indice.present = i;
			}

			if(indice.isSupport(graphics_mode)){
				break;
			}

			++i;
		}

		return indice;
	}
}

#endif