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

		VkDebugUtilsMessengerEXT debugMessenger;
		private :
		VkInstance instance;
		const vector<const char *> validations;
		const vector<const char *> extensions;
		
		public :
		explicit Engine(const string _app_name, 
						const string _engine_name, 
						const vector< const char* > _extensions, 
						const vector< const char* > _validations
		);
		void init();
		operator VkInstance() const {
			return instance;
		}
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		};

		void destroy();

		private :
		void createInstance();
		bool checkValidationSupport();
		bool checkDeviceFeaturesSupport();
		void setupDebugMessenger();
	};
}

#endif