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
	setPhysicalDevice(pdevices[id]);
}

void PhysicalDevice::prepareDeviceProperties()
{
	vkGetPhysicalDeviceProperties(device, &properties);
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
		if(prop.queueFlags == VK_QUEUE_GRAPHICS_BIT){
			indices.graphics = i;
		}
		if(prop.queueFlags == VK_QUEUE_COMPUTE_BIT){
			indices.compute = i;
		}
		if(prop.queueFlags == VK_QUEUE_TRANSFER_BIT){
			indices.transfer = i;
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

Engine* PhysicalDevice::getEngine(){
	return engine;
}

}
#endif