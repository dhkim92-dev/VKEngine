#ifndef __VK_PHYSICALDEVICE_CPP__
#define __VK_PHYSICALDEVICE_CPP__

#include "vk_physicaldevice.h"

namespace VKEngine{

PhysicalDevice::PhysicalDevice(Engine *engine)
{
	this->engine = engine;
}

void PhysicalDevice::init()
{
	prepareDeviceProperties();
	prepareMemoryProperties();
	prepareDeviceFeatures();
	prepareDeviceExtensions();
}


void PhysicalDevice::useGPU(int id)
{
	if(device != VK_NULL_HANDLE){
		LOG("Device already selected. you can not change after you first set Physical Device\n");
		return ;
	}
	vector<VkPhysicalDevice> pdevices=enumerateGPU(engine->getInstance());
	if(pdevices.size() == 0){
		std::runtime_error("Your System has no Vulkan support GPUs.\n");
	}
	if(id > pdevices.size()){
		LOG("Given GPU id is invalid. first gpu will be selected.");
		id = 0;
	}
	device = pdevices[id];
}

void PhysicalDevice::useSwapchain(bool value)
{
	use_swapchain = value;
}

// bool PhysicalDevice::isSwapchainEnabled()
// {
	// return use_swapchain;
// }

void PhysicalDevice::prepareDeviceProperties()
{
	vkGetPhysicalDeviceProperties(device, &properties);
}

void PhysicalDevice::prepareMemoryProperties()
{
	vkGetPhysicalDeviceMemoryProperties(device, &mem_properties);
}

void PhysicalDevice::prepareDeviceFeatures()
{
	vkGetPhysicalDeviceFeatures(device, &features);
}

void PhysicalDevice::prepareDeviceExtensions()
{
	uint32_t nr_exts;
	
	vector<VkExtensionProperties> props;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &nr_exts, nullptr);
	props.resize(nr_exts);
	LOG("This GPU has %d device extensions.\n", nr_exts);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &nr_exts, props.data());

	if(use_swapchain)
	{
		bool found = false;
		for(int i = 0 ; i < props.size() ; ++i){
			if(strcmp( props[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0){
				found = true;
				support_device_extensions.push_back(props[i].extensionName);
				break;
			}
		}
		if(!found){
			LOG("Your GPU doesn't support VK_KHR_SWAPCHAIN_EXTENSION.\n");
			exit(EXIT_FAILURE);
		}
	}

	for(int i = 0 ; i < props.size() ; i++){
		if(strcmp(props[i].extensionName, "VK_KHR_portability_subset") == 0 )
		{
			support_device_extensions.push_back(props[i].extensionName);
			break;
		}
	}

	for(int i = 0 ; i< nr_exts ; i++){
		support_device_extensions.push_back(props[i].extensionName);
	}
}


bool PhysicalDevice::isSupportDeviceExtension(const char *ext_name){
	//return (std::find(support_device_extensions.begin(), support_device_extensions.end(), ext_name) != support_device_extensions.end());
	for(int i = 0 ; i < support_device_extensions.size() ; i++){
		if(strcmp(support_device_extensions[i],ext_name)==0){
			return true;
		}
	}

	return false;
}

VkBool32 PhysicalDevice::findQueueFamilyIndice(QueueFamilyIndice *pindice, VkQueueFlags flags)
{
	QueueFamilyIndice indices;
	vector<VkQueueFamilyProperties> queue_properties = enumerateQueueFamilyProperties(device);
	VkBool32 found = VK_FALSE;

	if(queue_properties.size() <= 0){
		throw runtime_error("VkPhysicalDevice has no queue_properties.\n");
	}
	int i = 0 ;
	for(int i = 0 ;  i < queue_properties.size() ; i++){
		VkQueueFamilyProperties prop = queue_properties[i];
		if((prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)==VK_QUEUE_GRAPHICS_BIT){
			if(!indices.graphics.has_value()) {
				indices.graphics = i;
				indices.present = i;
				LOG("graphics queue index : %d\n", i);
				continue;
			}
		}
		if((prop.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT){
			if(!indices.compute.has_value()) {
				indices.compute = i;
				LOG("compute queue index : %d\n", i);
				continue;
			}
		}
		if((prop.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT){
			if(!indices.transfer.has_value()) {
				LOG("transfer queue index : %d\n", i);
				indices.transfer = i;
				continue;
			}
		}

		if(indices.isSupport(flags)){
			found = VK_TRUE;
			pindice->graphics = indices.graphics;
			pindice->transfer = indices.transfer;
			pindice->compute = indices.compute;	
			break;
		}
	}

	return found;
}

uint32_t PhysicalDevice::getMemoryType(uint32_t type, VkMemoryPropertyFlags flags, VkBool32* found)
{
	for(uint32_t i = 0 ; i < mem_properties.memoryTypeCount ; ++i){
		if((type&1)==1){
			VkMemoryPropertyFlags _flags = mem_properties.memoryTypes[i].propertyFlags;
			if((_flags & flags)==flags){
				if(found){
					*found = VK_TRUE;
				}
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

// setter
void PhysicalDevice::setPhysicalDevice(VkPhysicalDevice pdevice)
{
	if(device != VK_NULL_HANDLE){
		return ;
	}

	device = pdevice;
}

// getter

VkPhysicalDevice PhysicalDevice::getPhysicalDevice(){
	return device;
}

VkPhysicalDeviceFeatures PhysicalDevice::getDeviceFeatures(){
	return features;
}

VkPhysicalDeviceProperties PhysicalDevice::getDeviceProperties(){
	return properties;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::getMemoryProperties(){
	return mem_properties;
}


vector<const char*> PhysicalDevice::getSupportedExtensions() 
{
	return support_device_extensions;
}

Engine* PhysicalDevice::getEngine(){
	return engine;
}

}
#endif