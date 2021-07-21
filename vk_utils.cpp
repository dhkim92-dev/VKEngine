#ifndef __VK_UTILS_CPP__
#define __VK_UTILS_CPP__

#include <vulkan/vulkan.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <set>
#include "vk_utils.h"

using namespace std;


namespace VKEngine{
	vector<VkPhysicalDevice> enumerateGPU(VkInstance instance){
		uint32_t size;
		vector<VkPhysicalDevice> gpus;
		vkEnumeratePhysicalDevices(instance, &size, nullptr);
		if(size > 0){
			gpus.resize(size);
			vkEnumeratePhysicalDevices(instance, &size, gpus.data());
		}
		return gpus;
	}

	vector<VkLayerProperties> enumerateValidations(){
		uint32_t size;
		vector<VkLayerProperties> validations;
		vkEnumerateInstanceLayerProperties(&size, nullptr);
		if(size > 0){
			validations.resize(size);
			vkEnumerateInstanceLayerProperties(&size, validations.data());
		}
		return validations;
	}

	vector<VkQueueFamilyProperties> enumerateQueueFamilyProperties(VkPhysicalDevice gpu){
		uint32_t size;
		vector<VkQueueFamilyProperties> properties;

		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &size, nullptr);
		if(size > 0){
			properties.resize(size);
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &size, properties.data());
		}

		return properties;
	}

	SwapChainSupportDetail querySwapChainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface){
		SwapChainSupportDetail detail;
		uint32_t nr_formats;
		uint32_t nr_present_modes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &detail.capabilities);
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &nr_formats, nullptr);
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &nr_present_modes, nullptr);

		if(nr_formats > 0)
			detail.formats.resize(nr_formats);
		if(nr_present_modes > 0)
			detail.present_modes.resize(nr_present_modes);

		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &nr_formats, detail.formats.data());
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &nr_present_modes, detail.present_modes.data());
		return detail;		
	}

};

#endif