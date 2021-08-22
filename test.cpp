#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include "vk_core.h"
#include "vk_application.h"
#include "cstdio"

using namespace std;
using namespace VKEngine;

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

class MatmulTest{
	struct KernelElement{
		string name;
		Kernel *kernel;
	};
	struct Shape{
		uint32_t M = 1;
		uint32_t N = 1;
		uint32_t K = 1;
	};

	public : 
	Context *context = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	unordered_map<string, Kernel*> kernels;
	CommandQueue *command_queue;
	vector<Buffer *> d_buffers;
	VkDescriptorPool descriptor_pool;
	array<VkDescriptorSet, 3> descriptor_sets;
	VkPipelineCache cache;

	MatmulTest(Context *_context){
		context = _context;
		device = VkDevice(*context);
		LOG("ComnmandQueue Created!\n");
		command_queue = new CommandQueue(context, VK_QUEUE_COMPUTE_BIT);
		LOG("ComnmandQueue Created!\n");
	}

	void init(){
		VkPipelineCacheCreateInfo cache_CI = {};
		cache_CI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(device, &cache_CI, nullptr, &cache));
	}

	void createKernel(const string name, const string file_path){
		LOG("createKernel::file_path : %s\n", file_path.c_str());
		Kernel *kernel = new Kernel(context, file_path);
		kernels.insert({name, kernel});
	}

	void setupDescriptorPool(){
		std::array <VkDescriptorPoolSize, 1> pool_sizes={
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , 12}
		};
		VkDescriptorPoolCreateInfo info = infos::descriptorPoolCreateInfo(1, pool_sizes.data(), 4);
		VK_CHECK_RESULT(vkCreateDescriptorPool(VkDevice(*context), &info, nullptr, &descriptor_pool));
		LOG("setupDescriptorPool done\n");
	}

	void setKernelLayout(){
		kernels["matmul"]->setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3)
		});
	}

	void buildKernels(){
		for(auto iter = kernels.begin() ; iter != kernels.end() ; ++iter){
			iter->second->build(cache);
		}
	}

	void setupDescriptorSets(){
		kernels["matmul"]->allocateDescriptorSet(descriptor_pool);
	}

	void matmul(float *m1, float* m2, uint32_t M, uint32_t N, uint32_t K){
		float *output = new float[M*N];

		for(uint32_t i = 0 ; i < M*N ; ++i){
			output[i] = i * 1.0f;
		} 
		Buffer d_m1(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			sizeof(float) * M * K,
			nullptr
		);
		
		Buffer d_m2(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			sizeof(float) * N * K,
			nullptr
		);

		Buffer d_output(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			sizeof(float) * M * N,
			nullptr
		);

		Buffer d_shape(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			sizeof(uint32_t)*3,
			nullptr
		);
		Shape shape = {M, N, K};
		command_queue->enqueueCopy(m1, &d_m1, 0, 0, sizeof(float)*M*K);
		command_queue->enqueueCopy(m2, &d_m2, 0, 0, sizeof(float)*N*K);
		command_queue->enqueueCopy(&shape, &d_shape, 0, 0, sizeof(uint32_t)*3);

		kernels["matmul"]->setKernelArgs(
			{
				{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_m1.descriptor, nullptr},
				{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_m2.descriptor, nullptr},
				{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_output.descriptor, nullptr},
				{3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_shape.descriptor, nullptr}
			}
		);
		command_queue->ndRangeKernel(kernels["matmul"], {M, N, 1}, VK_TRUE);
		command_queue->enqueueCopy(&d_output, output, 0, 0, sizeof(float) * M * N);
		for(uint32_t i = 0 ; i < M ; ++i){
			for(uint32_t j = 0 ; j < N ; ++j){
				printf("%.3f ", output[i*N + j]);
			}
			printf("\n");
		}

		d_m1.destroy();
		d_m2.destroy();
		d_shape.destroy();
		d_output.destroy();
		delete [] output;
	}

	void batch(){
		float mat1[9] = {
			1.0f, 2.0f ,3.0f,
			2.0f, 3.0f, 4.0f,
			4.0f, 5.0f, 6.0f
		};
		float mat2[9] = {
			0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f
		};

		matmul(mat1, mat2, 3, 3, 3);
	}
	
	void run(){
		init();
		createKernel("matmul", "./shaders/matmul/matmul.comp.spv");
		setupDescriptorPool();
		setKernelLayout();
		buildKernels();
		setupDescriptorSets();
		batch();
	}

	~MatmulTest(){
		for(auto &iter : kernels){
			vkFreeDescriptorSets(device, descriptor_pool, 1, &iter.second->descriptors.set);
			iter.second->destroy();
		}

		delete command_queue;
		delete context;
	}
};


int main(int argc, char *argv[])	
{
	vector<const char*> instance_extensions(getRequiredExtensions());
	vector<const char *> validations={"VK_LAYER_KHRONOS_validation"};
	vector<const char *>device_extensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	string _name = "vulkan";
	string engine_name = "engine";
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
	VkSurfaceKHR surface;

	try {
		Engine engine(_name, engine_name, instance_extensions, validations);
		engine.init();
		glfwCreateWindowSurface(VkInstance(engine), window, nullptr, &surface);
		Context context(VkInstance(engine), 0, surface, VK_QUEUE_COMPUTE_BIT, device_extensions, validations);
		MatmulTest test(&context);
		test.run();
	}catch(std::runtime_error& e){
		cout << "error occured : " << e.what()  <<  "on File " << __FILE__ << " line : " << __LINE__ << "\n";
		exit(EXIT_FAILURE);
	};

	return 0;
}