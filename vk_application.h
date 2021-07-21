#ifndef __VK_APPLICATION_H__
#define __VK_APPLICATION_H__

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#if defined(GLFW_INCLUDE_VULKAN)	
#include <GLFW/glfw3.h>
#endif
#include "vk_core.h"

namespace VKEngine{
	class Application{
		public :
		vector<const char *>instance_extension_names;
		vector<const char *>device_extension_names;
		vector<const char *>validation_names;
		#if defined(GLFW_INCLUDE_VULKAN)
		GLFWwindow *window;
		#endif
		int height, width;
		explicit Application(
			string app_name, string engine_name,
			int _height, int _width, 
			vector<const char *>_instance_extension_names,
			vector<const char *> device_extension_names,
			vector<const char *>_validation_names);
		~Application();
		void initVulkan();
		void run();
		
		protected :
		Engine *engine;
		Context *context;
		VkSurfaceKHR surface;
		void destroy();
		virtual void initWindow(){LOG("Application::initWindow()\n");};
		virtual void createSurface(){LOG("Application::initSurface()\n");};
		virtual void setupCommandQueue(){LOG("Application::setupCommandQueue()\n");};
		virtual void mainLoop(){};
		virtual void setupDepthStencil(){};
		virtual void setupFramebuffer(){};
	};
};
#endif