#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include "vk_core.h"
#include "vk_application.h"
using namespace std;
using namespace VKEngine;

class App : public VKEngine::Application{
	public :
	explicit App(string app_name, string engine_name, int h, int w, const vector<const char*>exts, const vector<const char *>valids) : Application(app_name, engine_name, h, w, exts, valids){};
	protected:
	virtual void initWindow(){
		glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
		mainLoop();
		destroy();
	}

	virtual void createSurface(){
		VK_CHECK_RESULT(glfwCreateWindowSurface(VkInstance(*engine), window, nullptr, &surface));
	}

	virtual void mainLoop(){
		while(!glfwWindowShouldClose(window)){
			glfwPollEvents();
		}
	}
	public:
	void run(){
		Application::run();
		int a ;
		return ;
	}
};

vector<const char *> getRequiredExtensions(  ){
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	return extensions;
}

int main(int argc, char *argv[])	
{
	vector<const char *> extensions= getRequiredExtensions();
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	vector<const char *> validations={"VK_LAYER_KHRONOS_validation"};
	string _name = "vulkan";
	string engine_name = "engine";
	
	App app(_name, engine_name, 600, 800, extensions, validations);
	try {
		app.run();
	}catch(std::runtime_error& e){
		cout << "error occured : " << e.what()  << "\n";
		exit(EXIT_FAILURE);
	}

	return 0;
}