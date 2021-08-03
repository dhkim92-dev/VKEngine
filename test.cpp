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
#include "cstdio"

using namespace std;
using namespace VKEngine;

void printBuffer(Buffer *buffer){
	VkDeviceSize size = VkDeviceSize(*buffer);
	float *data = new float[static_cast<uint32_t>( size ) ];
	buffer->copyTo(data, size);


	for(int i = 0 ; i < 3 ; i++){
		for(int j = 0 ; j < 3 ; j++){
			printf("%.1f ", data[i*3+j]);
		}
		printf("\n");
	}

	delete [] data;
}

void printPtr(float *ptr){
	for(int i = 0 ; i < 3 ; i++){
		for(int j = 0 ; j < 3 ; j++){
			printf("%.1f ", ptr[i*3+j]);
		}
		printf("\n");
	}

}

vector<const char *> getRequiredExtensions(  ){
	glfwInit();
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if(validationEnable) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	glfwTerminate();
	return extensions;
}

class App : public VKEngine::Application{
	public :
	explicit App(string app_name, string engine_name, int h, int w, vector<const char*>instance_exts, vector<const char*>device_exts , vector<const char *>valids) : Application(app_name, engine_name, h, w, instance_exts, device_exts, valids){
	};
	protected:
	
	virtual void initWindow(){
		LOG("App Init Window\n");
		glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
	}

	virtual void createSurface(){
		LOG("createSurface()\n");
		VK_CHECK_RESULT(glfwCreateWindowSurface(VkInstance(*engine), window, nullptr, &surface));
	}

	virtual void mainLoop(){
		float data[9] = {
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f
		};

		float output[9] = {};
		Buffer h_output(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 36, nullptr);
		Buffer d_output(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 9*4, nullptr);
		graphics_queue->enqueueCopy(data, &d_output, 0, 0, 36);
		graphics_queue->enqueueCopy(&d_output, output, 0, 0, 36);
		graphics_queue->enqueueCopy(&d_output, &h_output, 0, 0, 36);
		//printBuffer(&h_output);
		printPtr(output);

		h_output.destroy();
		d_output.destroy();

		while(!glfwWindowShouldClose(window)){
			glfwPollEvents();
		}
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	public:
	void run(){
		Application::init();
		mainLoop();
	}
};


int main(int argc, char *argv[])	
{
	vector<const char*> instance_extensions(getRequiredExtensions());
	vector<const char *> validations={"VK_LAYER_KHRONOS_validation"};
	vector<const char *>device_extensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	string _name = "vulkan";
	string engine_name = "engine";

	try {
		App app(_name, engine_name, 600, 800, instance_extensions, device_extensions , validations);
		app.run();
	}catch(std::runtime_error& e){
		cout << "error occured : " << e.what()  <<  "on File " << __FILE__ << " line : " << __LINE__ << "\n";
		exit(EXIT_FAILURE);
	};

	
	return 0;
}