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
#include "vk_validations.h"

using namespace std;

namespace VKEngine{
class Engine {
	// fields
	public :
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	private :
	VkInstance instance = VK_NULL_HANDLE;
	vector<VkQueueFamilyProperties> queue_family_properties;
	string name;
	vector<const char *> validations;
	vector<const char *> instance_extensions;
	vector<const char*>  device_extensions;
	bool debug = false;

	//methods
	public :
	explicit Engine(string _name);
	explicit Engine(string _name,
					vector< const char* > instance_extensions, 
					vector <const char*> device_extensions,
					vector< const char* > _validations
	);
	~Engine();

	void init();
	void destroy();
	void setDebug(bool value);
	void createInstance();

	//setters
	void setInstance(VkInstance instance);
	void setQueueFamilyProperties(vector<VkQueueFamilyProperties> properties);
	void setValidationLayers(vector<const char*> vlayers);
	void setInstanceExtensions(vector<const char*> extensions);
	void setDeviceExtensions(vector<const char *> extensions);

	bool isValidated();
	//getters
	vector<VkQueueFamilyProperties> getQueueFamilyProperties();
	vector<const char *> getDeviceExtensions();
	vector<const char *> getInstanceExtensions();
	vector<const char *> getValidationLayers();

	VkInstance getInstance() const {
		return this->instance;
	}

	private :
	void _createInstance();
	bool checkValidationSupport();
	bool checkDeviceFeaturesSupport();
	void setupDebugMessenger();
};
}

#endif