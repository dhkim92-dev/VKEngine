#ifndef __VK_ENGINE_H__
#define __VK_ENGINE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cstring>
#include "vk_utils.h"
#include "vk_infos.h"

using namespace std;

#ifdef DEBUG
const bool validationEnable = true;
#else
const bool validationEnable = false;
#endif

namespace VKEngine{

		class Engine {
		//variables
		public :
		const string app_name;
		const string engine_name;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
		vector<VkQueueFamilyProperties> queue_family_properties;
		const vector<const char *> validations;
		const vector<const char *> instance_extensions;
		const vector<const char*>  device_extensions;

		private :
		VkInstance instance = VK_NULL_HANDLE;
				
		public :
		explicit Engine(const string _app_name, 
						const string _engine_name, 
						const vector< const char* > instance_extensions, 
						const vector <const char*> device_extensions,
						const vector< const char* > _validations
		);
		~Engine();
		void init();
		void destroy();
		/*
		template<typename VkInstance> operator VkInstance() const {
			return instance;
		}
		*/

		VkInstance getInstance() const {
			return this->instance;
		}

		private :
		void createInstance();
		bool checkValidationSupport();
		bool checkDeviceFeaturesSupport();
		void setupDebugMessenger();
		
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		};
	};
}

#endif