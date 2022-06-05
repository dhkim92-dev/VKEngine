#ifndef __VK_ENGINE_CPP__
#define __VK_ENGINE_CPP__

#include "vk_engine.h"
using namespace std;

namespace VKEngine{
	Engine::Engine(string name)
	{
		this->name = name;
	}

	Engine::Engine(string _name,
				   vector<const char * > _instance_extensions,
				   vector<const char*> _device_extensions,
				   vector<const char * > _validations
	) : name(_name), instance_extensions(_instance_extensions), device_extensions(_device_extensions) ,validations(_validations) {};

	Engine::~Engine(){
		destroy();
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
		setupDebugMessenger();
	}

	void Engine::createInstance(){
		if(instance != VK_NULL_HANDLE)
		{
			_createInstance();
		}
	}

	void Engine::_createInstance(){
		if(debug){
			if(!checkValidationSupport()){
				LOG("build failed\n");
				throw std::runtime_error("validation layer requested but not available!");
			}
		}
		VkApplicationInfo app_info = infos::appCreateInfo();
		app_info.pApplicationName = name.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = name.c_str();
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo instance_info = infos::instanceCreateInfo();
		instance_info.pApplicationInfo = &app_info;
		instance_info.ppEnabledExtensionNames = instance_extensions.data();
		instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
		VkDebugUtilsMessengerCreateInfoEXT debug_info{};

		if(debug){
			LOG("Enigne::createInstance() => validation setting\n");
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
        if (!debug) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
		LOG("Engine::setupDebugMessenger()\n");
        createInfo = infos::debugMessengerCreateInfo();
		LOG("Engine::setupDebugMessenger::createInfo()\n");
		createInfo.pfnUserCallback = debugCallback;

		LOG("Engine::setupDebugMessenger::debugCallback()\n");
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
		LOG("Engine::setupDebugMessenger::createDebugUtilsMessengerEXT()\n");
    }

	// setter
	void Engine::setDebug(bool value){
		debug = value;
	}

	void Engine::setInstance(VkInstance instance){
		this->instance = instance;
	}

	void Engine::setQueueFamilyProperties(vector<VkQueueFamilyProperties> properties){
		queue_family_properties.assign(properties.begin(), properties.end());
	}

	void Engine::setValidationLayers(vector<const char*> vlayers){
		validations.assign(vlayers.begin(), vlayers.end());
	}

	void Engine::setDeviceExtensions(vector<const char *> extensions){
		device_extensions.assign(extensions.begin(), extensions.end());
	}

	// getter

	vector<VkQueueFamilyProperties> Engine::getQueueFamilyProperties()
	{
		return queue_family_properties;
	}

	vector<const char *> Engine::getDeviceExtensions()
	{
		return device_extensions;
	}

	vector<const char *> Engine::getInstanceExtensions()
	{
		return instance_extensions;
	}

	vector<const char *> Engine::getValidationLayers()
	{
		return validations;
	}

}

#endif