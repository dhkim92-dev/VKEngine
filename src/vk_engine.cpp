#ifndef __VK_ENGINE_CPP__
#define __VK_ENGINE_CPP__

#include "vk_engine.h"
using namespace std;

namespace VKEngine{
	
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	    if (func != nullptr) {
	        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	    } else {
	        return VK_ERROR_EXTENSION_NOT_PRESENT;
	    }
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) 
			func(instance, debugMessenger, pAllocator);
	}

	
	Engine::Engine(const string _app_name, 
				   const string _engine_name,
				   const vector<const char * > _instance_extensions,
				   vector<const char*> _device_extensions,
				   const vector<const char * > _validations
				   ) : app_name(_app_name), engine_name(_engine_name), instance_extensions(_instance_extensions), device_extensions(_device_extensions) ,validations(_validations) {};

	Engine::~Engine(){
		LOG("Engine::~Engine()\n");
		destroy();
		LOG("Engine::~Engine() end\n");
	}

	void Engine::destroy(){
		LOG("Engine::destroy()\n");
		if(debugMessenger != VK_NULL_HANDLE){
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		if(instance){
			vkDestroyInstance(instance, nullptr);
			instance = VK_NULL_HANDLE;
		}
		LOG("Engine::destroy() end\n");
	}

	void Engine::init(){
		LOG("Engine::init() createInstance()\n");
		createInstance();
		LOG("Engine::init() setupDebugMessenger()\n");
		setupDebugMessenger();
	}

	void Engine::createInstance(){
		if(validationEnable && !checkValidationSupport()){
			LOG("build failed\n");
			throw std::runtime_error("validation layer requested but not available!");
		}
		VkApplicationInfo app_info = infos::appCreateInfo();
		app_info.pApplicationName = app_name.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = engine_name.c_str();
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo instance_info = infos::instanceCreateInfo();
		instance_info.pApplicationInfo = &app_info;
		instance_info.ppEnabledExtensionNames = instance_extensions.data();
		instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
		VkDebugUtilsMessengerCreateInfoEXT debug_info{};

		if(validationEnable){
			instance_info.ppEnabledLayerNames = validations.data();
			instance_info.enabledLayerCount = static_cast<uint32_t>(validations.size());
			debug_info = infos::debugMessengerCreateInfo();
			debug_info.pfnUserCallback = debugCallback;
			instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_info;
		}else{
			instance_info.enabledLayerCount=0;
			instance_info.pNext = nullptr;
		}

		VK_CHECK_RESULT( vkCreateInstance(&instance_info, nullptr, &instance) );
	}

	bool Engine::checkValidationSupport(){
		vector<VkLayerProperties> all_validations =  enumerateValidations();
		bool res = true;
		for(const char* valid_name : validations){
			bool check = false;
			for(VkLayerProperties layer : all_validations){
				if( !strcmp(valid_name, layer.layerName) ){
					check = true;
					break;
				}
			}
			res &= check;
		}

		return res;
	}
	 void Engine::setupDebugMessenger() {
        if (!validationEnable) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        createInfo = infos::debugMessengerCreateInfo();
		createInfo.pfnUserCallback = debugCallback;
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
}

#endif