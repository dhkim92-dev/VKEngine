#ifndef __VK_APPLICATION_CPP__
#define __VK_APPLICATION_CPP__

#include "vk_application.h"

using namespace std;

namespace VKEngine{
	Application::Application(
		string app_name, string engine_name,
		int _height, int _width, 
		const vector<const char *> instance_extensions,
		const vector<const char *> validations)
	{
		height = _height;
		width = _width;
		engine = new Engine(app_name, engine_name, instance_extensions, validations);
	}	

	void Application::initVulkan(){
		engine->init();
		VkInstance instance = VkInstance(engine);
		context = new Context(instance, 0, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);

	}
	
	void Application::run(){
		initWindow();
		initVulkan();
	}	

	Application::~Application(){
		destroy();
	}

	void Application::destroy(){
		delete context;
		delete engine;
	}
	void Application::initWindow() {}
	void Application::createSurface() {}
	void Application::mainLoop(){}
}
#endif