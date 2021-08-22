#ifndef __VK_UTILS_CPP__
#define __VK_UTILS_CPP__

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

	VkBool32 getDepthFormat(VkPhysicalDevice gpu, VkFormat *format){
		vector<VkFormat> depth_formats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};
		for(VkFormat depth_format : depth_formats){
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(gpu, depth_format, &properties);
			if(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
				*format = depth_format;
				return VK_TRUE;
			}
		}
		return VK_FALSE;
	}
	
	vector<VkImage> getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain){
		uint32_t nr_images;
		vector<VkImage> images; 
		vkGetSwapchainImagesKHR(device, swapchain, &nr_images, nullptr);
		if(nr_images > 0){
			images.resize(nr_images);
			vkGetSwapchainImagesKHR(device, swapchain, &nr_images, images.data());
		}
		return images;
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

	uint32_t getMemoryType(VkPhysicalDevice gpu, VkDevice device, uint32_t type, VkMemoryPropertyFlags properties, VkBool32 *mem_found){
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(gpu, &mem_properties);

		for(uint32_t i = 0 ; i < mem_properties.memoryTypeCount ; ++i){
			if((type & 1) == 1){
				if( (mem_properties.memoryTypes[i].propertyFlags & properties)  == properties ){
					if(mem_found){
						*mem_found = true;
					}

					return i;
				}
			}
			type>>=1;
		}

		if(mem_found){
			*mem_found = false;
			return 0;
		}else{
			std::runtime_error("can not find a matching memory type!\n");
		}
	}

	VkShaderModule loadShader(const string &file_path, VkDevice device){
		LOG("read file path : %s\n", file_path.c_str());
		VkShaderModule shader_module = VK_NULL_HANDLE;
		std::ifstream is(file_path, std::ios::binary | std::ios::in | std::ios::ate);
			if (is.is_open())		{
				size_t size = is.tellg();
				is.seekg(0, std::ios::beg);
				char* shader_code = new char[size];
				is.read(shader_code, size);
				is.close();
				LOG("shader read info : %s\n", file_path.c_str());
				assert(size > 0);
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = size;
				moduleCreateInfo.pCode = (uint32_t*)shader_code;
				VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shader_module));
				delete[] shader_code;
			}
			else{
				std::cerr << "Error: Could not open shader file \"" << file_path << "\"" << "\n";
			}
		return shader_module;
	}
};

#endif