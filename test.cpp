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


struct {
	struct{
		size_t x,y,z;
	}size;
	float isovalue;
	float *data;
	string file_path;
	uint32_t result;
}Volume;

void loadVolume(string file_path, void *data){
	printf("load volume!\n");
	std::ifstream is(file_path, std::ios::binary | std::ios::in | std::ios::ate);
	if(is.is_open()){
		size_t size = is.tellg();
		assert(size > 0);
		is.seekg(0, std::ios::beg);
		is.read((char *)data, size);
	}else{
		cerr << "fail to read volume data. check it first\n";
	}	
};

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

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	
	static vector<VkVertexInputBindingDescription> vertexInputBinding(){
		vector <VkVertexInputBindingDescription> bindings;
		VkVertexInputBindingDescription binding = {};
		binding.binding = 0;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = sizeof(Vertex);
		bindings.push_back(binding);
		return bindings;
	}

	static vector<VkVertexInputAttributeDescription> vertexInputAttributes(){
		vector<VkVertexInputAttributeDescription> attributes(2);
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[0].offset = offsetof(Vertex, pos);

		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].offset = offsetof(Vertex, color);
		return attributes;
	}
};


struct RenderObject{
	struct {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	}matrices;
	Buffer *ubo = nullptr;
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	Program *program = nullptr;
};

class App : public VKEngine::Application{
	public :

	explicit App(string app_name, string engine_name, int h, int w, vector<const char*>instance_exts, vector<const char*>device_exts , vector<const char *>valids) : Application(app_name, engine_name, h, w, instance_exts, device_exts, valids){
	};

	vector<RenderObject> render_objects;
	unordered_map<string, Program*> programs;
	Kernel volume_test, edge_test, cube_test;

	struct{
		struct{
			Buffer raw;
			Buffer iso_value;
		}host;

		struct{
			Buffer iso_value;
			Buffer raw;
			Buffer e_test;
			Buffer v_test;
			Buffer c_test;
			Buffer vertices;
			Buffer indices;
		}device;

		void destroy(){
			host.raw.destroy();
			host.iso_value.destroy();
			device.raw.destroy();
			device.v_test.destroy();
			//device.e_test.destroy();
		};
	}volume;

	struct {
		Kernel volume_test;
		VkDescriptorPool pool = VK_NULL_HANDLE;
		VkCommandBuffer command_buffer=VK_NULL_HANDLE;
	}compute;

	public :
	~App(){
		VkDevice device = VkDevice(*context);
		VkCommandPool command_pool = VkCommandPool(*compute_queue);
		volume.destroy();
		vkDestroyDescriptorPool(device, compute.pool, nullptr);
		vkFreeCommandBuffers(device, command_pool, 1, &compute.command_buffer);
	}

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
		while(!glfwWindowShouldClose(window)){
			glfwPollEvents();
			draw();
		}
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void draw(){
		render();
	}

	void prepareComputeBuffers(){
		size_t volume_size = Volume.size.x * Volume.size.y * Volume.size.z;
		volume.host.raw.create(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
						 	   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						  	   volume_size * sizeof(float), Volume.data);
		volume.device.raw.create(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, volume_size * sizeof(float), nullptr);
		volume.device.v_test.create(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, volume_size * sizeof(uint32_t), nullptr);
		volume.host.iso_value.create(context, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
									sizeof(float), &Volume.isovalue);
		volume.host.iso_value.map(0, 4);
		compute_queue->enqueueCopy( &volume.host.raw, &volume.device.raw, 0, 0, volume_size*sizeof(float) );
	}

	void prepareComputeKernels(){
		VkDevice device = VkDevice(*context);
		vector<VkDescriptorPoolSize> pool_sizes({
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2),
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
		});
		VkDescriptorPoolCreateInfo pool_CI = infos::descriptorPoolCreateInfo( static_cast<uint32_t>(pool_sizes.size()), pool_sizes.data(), 1);
		VK_CHECK_RESULT(vkCreateDescriptorPool( device,  &pool_CI, nullptr, &compute.pool));
		compute.volume_test.create(context, "shaders/marching_cube/volume_test.comp.spv");
		compute.volume_test.setupDescriptorSetLayout(
			{
				infos::descriptorSetLayoutBinding( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , VK_SHADER_STAGE_COMPUTE_BIT,  0),
				infos::descriptorSetLayoutBinding( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , VK_SHADER_STAGE_COMPUTE_BIT,  1),
				infos::descriptorSetLayoutBinding( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2)
			}
		);
		compute.volume_test.allocateDescriptorSet( compute.pool );
		cout << "kernel build start\n";
		compute.volume_test.build(cache);
		cout << "kernel build done\n";
		compute.volume_test.setKernelArgs( { 
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &volume.device.raw.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &volume.device.v_test.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &volume.host.iso_value.descriptor, nullptr}
		});
		cout << "prepare compute kernel complete\n";
	}

	void buildComputeCommandBuffers(){
		compute.command_buffer = compute_queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		compute_queue->beginCommandBuffer(compute.command_buffer);
		//compute_queue->ndRangeKernel(&compute.volume_test, {256,256,256}, VK_TRUE);

		vkCmdBindPipeline(compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.volume_test.pipeline);
		vkCmdBindDescriptorSets(compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
								compute.volume_test.layout, 
								0, 1, &compute.volume_test.descriptors.set, 0, nullptr);
		vkCmdDispatch(compute.command_buffer, Volume.size.x, Volume.size.y, Volume.size.z);
		compute_queue->endCommandBuffer(compute.command_buffer);
	}
	
	void preparePrograms(){
		// LOG("-------------Test::preparePrograms() start------------------------\n");
		// Program *program = new Program(context);
		// program->attachShader("./shaders/cubes/cube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		// program->attachShader("./shaders/cubes/cube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		// program->setupDescriptorSetLayout({
		// 	infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1)
		// });
		// program->createDescriptorPool({
		// 	infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
		// });
		// auto attributes = Vertex::vertexInputAttributes();
		// auto bindings = Vertex::vertexInputBinding();
		// program->graphics.vertex_input = infos::vertexInputStateCreateInfo(attributes, bindings);
		// program->build(render_pass, cache);
		// programs.insert({"cube", program});
		// LOG("-------------Test::preparePrograms() end------------------------\n");
	}

	void prepareRenderObjects(){
	}

	void prepareCommandBuffer(){
		// std::array<VkClearValue, 2> clear_values{};
		// clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		// clear_values[1].depthStencil = {1.0f, 0};
		// draw_command_buffers.resize(swapchain.buffers.size());
		
		// VkRenderPassBeginInfo render_pass_BI = infos::renderPassBeginInfo();
		// render_pass_BI.clearValueCount = static_cast<uint32_t>(clear_values.size());
		// render_pass_BI.pClearValues = clear_values.data();
		// render_pass_BI.renderArea.offset = {0,0};
		// render_pass_BI.renderArea.extent.height = height;
		// render_pass_BI.renderArea.extent.width = width;
		// render_pass_BI.renderPass = render_pass;

		// for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
		// 	draw_command_buffers[i] = graphics_queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		// }
		
		// for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
			
			// graphics_queue->beginCommandBuffer(draw_command_buffers[i]);
			// render_pass_BI.framebuffer = framebuffers[i];
			// VkViewport viewport = infos::viewport(static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
			// VkRect2D scissor = infos::rect2D(width, height, 0, 0);
			// vkCmdBeginRenderPass(draw_command_buffers[i], &render_pass_BI, VK_SUBPASS_CONTENTS_INLINE);
			// vkCmdSetViewport(draw_command_buffers[i], 0, 1, &viewport);
			// vkCmdSetScissor(draw_command_buffers[i], 0, 1, &scissor);
			// vkCmdBindPipeline(draw_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, render_objects[0].program->pipeline);
			// VkBuffer vertex_buffer[] = {VkBuffer(*cube.vbo)};
			// VkBuffer indices_buffer[] = {VkBuffer(*cube.ibo)};
			// VkDeviceSize offsets[] = {0};
			// vkCmdBindVertexBuffers(draw_command_buffers[i], 0, 1, vertex_buffer, offsets);
			// vkCmdBindIndexBuffer(draw_command_buffers[i], VkBuffer(*cube.ibo), 0, VK_INDEX_TYPE_UINT16);
			// vkCmdBindDescriptorSets(draw_command_buffers[i], 
			// 						VK_PIPELINE_BIND_POINT_GRAPHICS,
			// 						render_objects[0].program->pipeline_layout, 
			// 						0, 
			// 						1, &render_objects[0].descriptor_set,
			// 						0, nullptr);
			// vkCmdDrawIndexed(draw_command_buffers[i], static_cast<uint32_t>(cube_indices.size()), 1, 0, 0, 0);
			// vkCmdEndRenderPass(draw_command_buffers[i]);
			// graphics_queue->endCommandBuffer(draw_command_buffers[i]);
		// }	
	}

	void executeCompute(){
		LOG("execute Compute\n");
		memcpy(volume.host.iso_value.data, &Volume.isovalue, sizeof(float));
		VkSubmitInfo submit_info = infos::submitInfo();
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &compute.command_buffer;
		vkQueueSubmit(VkQueue(*compute_queue), 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(VkQueue(*compute_queue));
		LOG("execute Compute done\n");
	}

	void copyResult(){
		size_t sz_mem = Volume.size.x * Volume.size.y * Volume.size.z * sizeof(bool);
		Buffer h_volume_test(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
			,VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sz_mem * sizeof(uint32_t), nullptr);
		compute_queue->enqueueCopy(&volume.device.v_test, &h_volume_test, 0, 0, sz_mem*sizeof(uint32_t));

		uint32_t nr_iter = Volume.size.x * Volume.size.y * Volume.size.z;
		uint32_t *vtest = new uint32_t[sz_mem];
		h_volume_test.copyTo( vtest, sz_mem  * sizeof(uint32_t));
		uint32_t vtest_sum = 0;

		for(uint32_t i = 0 ; i < nr_iter ; ++i){
			vtest_sum += (uint32_t)vtest[i];
		}

		printf("target_iso_value : %f\n vtest sum : %d\n", Volume.isovalue, vtest_sum);
		printf("CPU result : %d\n", Volume.result);

		delete[] vtest;
	}


	public:

	void prefixSumTest(){
		VkDevice device = VkDevice(*context);
		uint32_t src[512];
		uint32_t dst[512];
		uint32_t limit = 128;
		memset(dst, 0x00, sizeof(uint32_t)*128);
		for(uint32_t i = 0 ; i < 512 ; ++i){
			src[i] = 1;
		}

		Buffer h_src(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
							  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 512*4, (void *)src);
		Buffer d_src(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
					 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 512*4, nullptr);
		Buffer d_dst(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT , 
					 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 512*4, nullptr);
		Buffer d_limit(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 4, nullptr);

		compute_queue->enqueueCopy(&src, &d_src, 0, 0, 512 * 4);
		compute_queue->enqueueCopy(&dst, &d_dst, 0, 0, 512 * 4);
		compute_queue->enqueueCopy(&limit, &d_limit, 0, 0, 4);
		Kernel scan(context, "shaders/marching_cube/scan.comp.spv");
		KernelArgs args;

		VkDescriptorPool pool;
		vector<VkDescriptorPoolSize> pool_sizes = {
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,3),
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
		};
		VkDescriptorPoolCreateInfo pool_CI = infos::descriptorPoolCreateInfo(static_cast<uint32_t>(pool_sizes.size()), pool_sizes.data(), 1);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_CI, nullptr, &pool));
		scan.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			//infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2)
		});
		scan.allocateDescriptorSet(pool);
		
		scan.build(cache);

		printf("build done\n");
		scan.setKernelArgs({{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_src.descriptor,  nullptr},
							{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dst.descriptor,  nullptr},
							//{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dst.descriptor,  nullptr},
							{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &d_limit.descriptor, nullptr}});

		printf("ndRangeKernel ready\n");
		compute_queue->ndRangeKernel(&scan,{128,1,1}, VK_TRUE);
		compute_queue->enqueueCopy(&d_dst, &dst, 0, 0, 512 * 4);
		printf("[");
		for(int i = 0 ; i < 512 ; ++i){
			printf("%d, ", dst[i]);
		}
		printf("]\n");
	}

	void run(){
		Application::init();
		cout << "compute_queue : " << compute_queue << endl;
		prefixSumTest();
		//prepareComputeBuffers();
		//prepareComputeKernels();
		//buildComputeCommandBuffers();
		//executeCompute();
		//copyResult();
		
		
		
		// preparePrograms();
		// prepareRenderObjects();
		// prepareCommandBuffer();
		// mainLoop();
	}
};

int main(int argc, const char *argv[])	
{
	vector<const char*> instance_extensions(getRequiredExtensions());
	vector<const char *> validations={"VK_LAYER_KHRONOS_validation"};
	vector<const char *>device_extensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	string _name = "vulkan";
	string engine_name = "engine";
	
	string file_path(argv[1]);
	size_t x = atoi(argv[2]);
	size_t y = atoi(argv[3]);
	size_t z = atoi(argv[4]);
	float isovalue = atof(argv[5]);
	
	Volume.file_path = file_path;
	cout << "Volume file path set \n";
	Volume.size = {x,y,z};
	cout << "Volume size set \n";
	Volume.isovalue = isovalue;
	cout << "volume isovalue set done\n";

	Volume.data = new float[x*y*z];
	loadVolume(file_path, Volume.data);
	
	size_t nr_workitems = x*y*z;
	uint32_t sum = 0 ;
	for(uint32_t i = 0 ; i < nr_workitems ; ++i){
		sum = (Volume.data[i] > isovalue) ? sum + 1 : sum;
	}
	
	Volume.result = sum;


	try {
	    App app(_name, engine_name, 600, 800, instance_extensions, device_extensions , validations);
	    app.run();
	}catch(std::runtime_error& e){
		cout << "error occured : " << e.what()  <<  "on File " << __FILE__ << " line : " << __LINE__ << "\n";
		exit(EXIT_FAILURE);
	};


	delete [] Volume.data;
	return 0;
}
