#ifndef __VK_CONTEXT_CPP__
#define __VK_CONTEXT_CPP__

#include "vk_context.h"

namespace VKEngine{

Context::Context()
{

}

Context::Context(PhysicalDevice *pdevice, VkQueueFlags queue_flags) 
: pdevice(pdevice), queue_flags(queue_flags)
{
}

Context::~Context()
{
	destroy();
}

void Context::destroy()
{
	if(device != VK_NULL_HANDLE){
		vkDestroyDevice(device, nullptr);
	}
}

VkResult Context::initDevice()
{
	if(device != VK_NULL_HANDLE){
		LOG("Device already initialized");
		return VK_SUCCESS;
	}
	float queue_priorities = 1.0f;
	VkPhysicalDevice _pdevice = pdevice->getPhysicalDevice();
	VkBool32 found = pdevice->findQueueFamilyIndice(&indice, queue_flags);

	LOG("Queue Family Found : %d\n", found);
	if(found != VK_TRUE){
		if( (queue_flags & VK_QUEUE_GRAPHICS_BIT == VK_QUEUE_GRAPHICS_BIT) && !indice.graphics.has_value())
			std::runtime_error("This Device has no Suitable Queue Family. Maybe not support Vulkan API.");
		if(!indice.compute.has_value())
			indice.compute = indice.graphics.value();
		if(!indice.transfer.has_value())
			indice.transfer = indice.graphics.value();
	}

	vector<VkDeviceQueueCreateInfo> dqueue_CI = createQueueCI(&queue_priorities);
	Engine *engine = pdevice->getEngine();
	VkDeviceCreateInfo device_CI = infos::deviceCreateInfo();
	
	vector<const char*> device_extensions(engine->getDeviceExtensions());
	vector<const char*> validation_layers(engine->getValidationLayers());
	
	for(int i = 0 ; i < device_extensions.size() ; i++)
	{
		if(pdevice->isSupportDeviceExtension(device_extensions[i])){
			enabled_device_extensions.push_back(device_extensions[i]);
		}else{
			LOG("Device Extension %s not supported on this GPU.\n",device_extensions.data()+i);
		}
	}
	// enabled_device_extensions.push_back("VK_KHR_portability_subset");
	device_CI.enabledExtensionCount=0;
	device_CI.enabledLayerCount=0;
	

	// LOG("Queue CI generate\n");
	// LOG("validation layers size : %d\n", validation_layers.size());
	if(engine->isValidated()){
		// for(int i = 0 ; i < validation_layers.size() ; i++){
			// LOG("validation layer name : %s \n", (validation_layers[i]));
		// }

		device_CI.enabledLayerCount = validation_layers.size();
		device_CI.ppEnabledLayerNames = validation_layers.data();
	}
	LOG("enabled_device_extension size : %d\n", enabled_device_extensions.size());
	if(enabled_device_extensions.size() > 0){
		for(int i = 0 ; i <enabled_device_extensions.size() ; i++){
			LOG("device extension name : %s \n", (enabled_device_extensions[i]));
		}
		device_CI.enabledExtensionCount = engine->getDeviceExtensions().size();
		device_CI.ppEnabledExtensionNames = enabled_device_extensions.data();
	}
	device_CI.pEnabledFeatures = &device_features;
	device_CI.queueCreateInfoCount = dqueue_CI.size();
	device_CI.pQueueCreateInfos = dqueue_CI.data();

	VkResult result = vkCreateDevice(pdevice->getPhysicalDevice(),
		&device_CI,
		nullptr,
		&device
	);

	if(result != VK_SUCCESS){
		std::runtime_error("Context::initDevice() - Fail to create Vulkan device.\n");
	}

	LOG("init Device success\n");
	return result;
}

VkResult Context::useSurface(VkSurfaceKHR surface)
{	
	VkBool32 surface_support = VK_FALSE;

	if(surface == VK_NULL_HANDLE){
		return VK_SUCCESS;
	}
	VkPhysicalDevice gpu = pdevice->getPhysicalDevice();
	vector<VkQueueFamilyProperties> qf_properties = enumerateQueueFamilyProperties(gpu);
	for(uint32_t i = 0 ; i < qf_properties.size() ; ++i){
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &surface_support);
		if(surface_support == VK_SUCCESS){
			indice.present = i;
		}
	}

	if(surface_support == VK_FALSE){
		std::runtime_error("This GPU has no surface support.");
	}
	return VK_SUCCESS;
}

VkResult Context::createCommandPool(
	VkCommandPool *pool, 
	VkQueueFlags type, 
	VkCommandPoolCreateFlags flags)
{
	uint32_t queue_index = 255;
	switch(type){
		case VK_QUEUE_GRAPHICS_BIT : 
			queue_index = indice.graphics.value();
			break;
		case VK_QUEUE_COMPUTE_BIT :
			queue_index = indice.compute.value();
			break;
		case VK_QUEUE_TRANSFER_BIT : 
			queue_index = indice.transfer.value();
			break;
		default :
			break;
	}
	VkCommandPoolCreateInfo info = infos::commandPoolCreateInfo(queue_index, flags);
	
	return vkCreateCommandPool(device, &info, nullptr, pool);
}

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

void Context::destroyCommandPool(VkCommandPool *pool)
{
	if(*pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(device, *pool, nullptr);
}

void Context::destroySemaphore(VkSemaphore* semaphore)
{
	if(*semaphore != VK_NULL_HANDLE)
		vkDestroySemaphore(device, *semaphore, nullptr);
}

void Context::destroyEvent(VkEvent *event)
{
	if(*event != VK_NULL_HANDLE)
		vkDestroyEvent(device, *event, nullptr);
}

void Context::destroyFence(VkFence *fence)
{
	if(*fence != VK_NULL_HANDLE)
		vkDestroyFence(device, *fence, nullptr);
}

PhysicalDevice* Context::getPhysicalDevice() const {
	return pdevice;
}

VkDevice Context::getDevice() const {
	return device;
}

QueueFamilyIndice Context::getQueueFamily() const
{
	return indice;
}

void Context::setQueueFlags(VkQueueFlags flags)
{
	if(device != VK_NULL_HANDLE) return;
	queue_flags = flags;
	setupQueueFamily();
}

void Context::setPhysicalDevice(PhysicalDevice *device)
{
	pdevice = device;
}

void Context::setDevice(VkDevice device)
{
	device = device;
}

void Context::setDeviceFeatures(VkPhysicalDeviceFeatures features)
{
	device_features = features;
}

vector<VkDeviceQueueCreateInfo> Context::createQueueCI(float *priorities)
{
	vector<VkDeviceQueueCreateInfo> qCI;
	VkDeviceQueueCreateInfo info = infos::deviceQueueCreateInfo();
	info.queueCount=1;
	info.pQueuePriorities=	priorities;

	if(queue_flags & VK_QUEUE_GRAPHICS_BIT){
		info.queueFamilyIndex = indice.graphics.value();
		qCI.push_back(info);
	}

	if(queue_flags & VK_QUEUE_COMPUTE_BIT){
		info.queueFamilyIndex = indice.compute.value();
		qCI.push_back(info);
	}

	if(queue_flags & VK_QUEUE_TRANSFER_BIT){
		info.queueFamilyIndex = indice.transfer.value();
		qCI.push_back(info);
	}
	return  qCI;
}

void Context::setPresentQueueIndex(uint32_t value)
{
	indice.present = value;
}

void Context::setupQueueFamily(){
	VkBool32 found = pdevice->findQueueFamilyIndice(&indice, queue_flags);
	if(found != VK_TRUE){
		if((queue_flags & VK_QUEUE_GRAPHICS_BIT == VK_QUEUE_GRAPHICS_BIT) && !indice.graphics.has_value())
			std::runtime_error("This Device has no Suitable Queue Family. Maybe not support Vulkan API.");
		if(!indice.compute.has_value())
		{
			if(indice.graphics.has_value()){
				indice.compute = indice.graphics.value();
			}else{
				LOG("No Compute Queue Support.\n");
				exit(EXIT_FAILURE);
			}
		}
		if(!indice.transfer.has_value()){
			if(indice.graphics.has_value()){
				indice.transfer = indice.graphics.value();
			}else{
				LOG("No Transfer Queue Support.\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}
}
#endif