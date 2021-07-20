#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>
#include "vk_engine.h"

using namespace std;
using namespace VKEngine;

int main(int argc, char *argv[])
{
	try {
		vector<const char *> extensions={VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_MVK_macos_surface"};
		vector<const char *> validations={"VK_LAYER_KHRONOS_validation"};
		Engine engine("test", "engine", extensions, validations);
		engine.init();
	}catch(std::runtime_error& e){
		cout << "error occured : " << e.what()  << "\n";
		exit(EXIT_FAILURE);
	}
}