#ifndef __VK_VALIDATIONS_CPP__
#define __VK_VALIDATIONS_CPP__
#include "vk_validations.h"
#include "vk_utils.h"

namespace VKEngine
{

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    LOG("CreateDebugUtilsMessengerEXT::instance : %p\n", instance);
    LOG("pCreateInfo : %d\n",pCreateInfo->pfnUserCallback);
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {

        LOG("func not null : %p\n", instance);
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
        LOG("func null return VK_ERROR_EXSTENSION_NOT_PRESENT\n");
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance, 
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{		
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) 
		func(instance, debugMessenger, pAllocator);
}


};
#endif
