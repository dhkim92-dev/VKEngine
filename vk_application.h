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
		#if defined(GLFW_INCLUDE_VULKAN)
		GLFWwindow *window;
		#endif
		int height, width;
		explicit Application(
			string app_name, string engine_name,
			int _height, int _width, 
			const vector<const char *>instance_extensions,
			const vector<const char*>validations);
		~Application();
		void initVulkan();
		void run();
		
		protected :
		Engine *engine;
		Context *context;
		VkSurfaceKHR surface;
		void destroy();
		virtual void initWindow();
		virtual void createSurface();
		virtual void mainLoop();
	};
};
#endif