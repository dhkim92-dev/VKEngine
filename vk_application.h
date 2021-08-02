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
		uint32_t height, width;
		explicit Application(
			string app_name, string engine_name,
			uint32_t _height, uint32_t _width, 
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
		CommandQueue *graphics_queue, *compute_queue;
		SwapChain swapchain;
		void destroy();
		virtual void initWindow(){LOG("Application::initWindow()\n");};
		virtual void createSurface(){LOG("Application::initSurface()\n");};
		virtual void setupCommandQueue();
		virtual void mainLoop(){};
		virtual void setupDepthStencil(){};
		virtual void setupFramebuffer(){};
	};
};
#endif