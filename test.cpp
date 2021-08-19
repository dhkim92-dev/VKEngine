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

struct Cube{
	Buffer *vbo;
	Buffer *ibo;
}cube;

struct RenderObject{
	struct {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	}matices;
	Buffer *ubo = nullptr;
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	Program *program = nullptr;
};

vector<Vertex> cube_vertices = {
	
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, 
	{{0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.0f}},
	{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.5f, 0.5f}},
	{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 0.5f, 0.5f}},
	{{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.0f}}
	
	/*
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
	*/
	};

vector<uint16_t> cube_indices = {
	
	0,1,5,
	5,4,0,
	1,2,5,
	5,2,6,
	6,7,5,
	5,7,4,
	1,0,3,
	3,2,1,
	2,3,7,
	7,6,2,
	7,3,0,
	0,4,7
	
    /*0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4*/
};

class App : public VKEngine::Application{
	public :
	explicit App(string app_name, string engine_name, int h, int w, vector<const char*>instance_exts, vector<const char*>device_exts , vector<const char *>valids) : Application(app_name, engine_name, h, w, instance_exts, device_exts, valids){
	};

	vector<RenderObject> render_objects;
	unordered_map<string, Program*> programs;
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
			updateUniforms();
		}
		cleanup();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void draw(){
		render();
	}

	void preparePrograms(){
		LOG("-------------Test::preparePrograms() start------------------------\n");
		Program *program = new Program(context);
		program->attachShader("./shaders/cubes/cube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		program->attachShader("./shaders/cubes/cube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		program->setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1)
		});
		program->createDescriptorPool({
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
		});
		auto attributes = Vertex::vertexInputAttributes();
		auto bindings = Vertex::vertexInputBinding();
		program->graphics.vertex_input = infos::vertexInputStateCreateInfo(attributes, bindings);
		program->build(render_pass, cache);
		programs.insert({"cube", program});
		LOG("-------------Test::preparePrograms() end------------------------\n");
	}

	void prepareRenderObjects(){
		LOG("-----------------------Test::prepareRenderObjects() start-----------------------\n");
		uint32_t nr_cubes = 1;
		uint32_t sz_mvp = sizeof(render_objects[0].matices);	
		size_t sz_vertex = sizeof(Vertex) * cube_vertices.size();
		size_t sz_index = sizeof(uint16_t) * cube_indices.size();
		render_objects.resize(nr_cubes);
		LOG("sz_mvp : %d\nsz_vertex : %d\nsz_index: % d\n", sz_mvp, sz_vertex, sz_index);

		LOG("descriptorSet : %p \n", render_objects[0].descriptor_set);
		for(uint32_t i = 0 ; i < nr_cubes ; ++i){
			render_objects[i].program = programs["cube"];
			render_objects[i].matices.model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0, 0.0f));
			render_objects[i].matices.view = camera.matrices.view;
			render_objects[i].matices.proj = camera.matrices.proj;
			render_objects[i].ubo = new Buffer(context, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sz_mvp, &render_objects[i].matices);
			render_objects[i].program->allocDescriptorSet(&render_objects[i].descriptor_set, 0);
		}
		LOG("descriptorSet : %p \n", render_objects[0].descriptor_set);
		writeDescriptors();
		LOG("index buffer object : %p \n", cube.ibo);
		cube.vbo = new Buffer( context, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sz_vertex, nullptr);
		cube.ibo = new Buffer(context, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sz_index , nullptr);
		graphics_queue->enqueueCopy(cube_vertices.data(), cube.vbo, 0, 0, sz_vertex);
		graphics_queue->enqueueCopy(cube_indices.data(), cube.ibo, 0, 0, sz_index);

		LOG("index buffer object : %p \n", cube.ibo);
		LOG("Index Buffer Object address : %p \n", VkBuffer(*cube.ibo));
		LOG("-----------------------Test::prepareRenderObjects() end-----------------------\n");
	}

	void prepareCommandBuffer(){
		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clear_values[1].depthStencil = {1.0f, 0};
		draw_command_buffers.resize(swapchain.buffers.size());
		
		VkRenderPassBeginInfo render_pass_BI = infos::renderPassBeginInfo();
		render_pass_BI.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_BI.pClearValues = clear_values.data();
		render_pass_BI.renderArea.offset = {0,0};
		render_pass_BI.renderArea.extent.height = height;
		render_pass_BI.renderArea.extent.width = width;
		render_pass_BI.renderPass = render_pass;

		for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
			draw_command_buffers[i] = graphics_queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}

		for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
			graphics_queue->beginCommandBuffer(draw_command_buffers[i]);
			render_pass_BI.framebuffer = framebuffers[i];
			VkViewport viewport = infos::viewport(static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
			VkRect2D scissor = infos::rect2D(width, height, 0, 0);
			vkCmdBeginRenderPass(draw_command_buffers[i], &render_pass_BI, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(draw_command_buffers[i], 0, 1, &viewport);
			vkCmdSetScissor(draw_command_buffers[i], 0, 1, &scissor);
			vkCmdBindPipeline(draw_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, render_objects[0].program->pipeline);
			VkBuffer vertex_buffer[] = {VkBuffer(*cube.vbo)};
			VkBuffer indices_buffer[] = {VkBuffer(*cube.ibo)};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(draw_command_buffers[i], 0, 1, vertex_buffer, offsets);
			vkCmdBindIndexBuffer(draw_command_buffers[i], VkBuffer(*cube.ibo), 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(draw_command_buffers[i], 
									VK_PIPELINE_BIND_POINT_GRAPHICS,
									render_objects[0].program->pipeline_layout, 
									0, 
									1, &render_objects[0].descriptor_set,
									0, nullptr);
			vkCmdDrawIndexed(draw_command_buffers[i], static_cast<uint32_t>(cube_indices.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(draw_command_buffers[i]);
			graphics_queue->endCommandBuffer(draw_command_buffers[i]);
		}	
	}

	void cleanupRenderObjects(){
		VkDevice device = VkDevice(*context);
		if(render_objects[0].descriptor_set != VK_NULL_HANDLE)
			render_objects[0].program->releaseDescriptorSet(&render_objects[0].descriptor_set);
		render_objects[0].program = nullptr;

		delete cube.vbo;
		delete cube.ibo;
		LOG("cleanup Render objects\n");
	}

	void cleanupPrograms(){
		LOG("cleanup Programs\n");
		for(auto &program : programs){
			program.second->destroy();
		}
	}

	void cleanup(){
		cleanupRenderObjects();
		cleanupPrograms();
	}

	void writeDescriptors(){
		LOG("Test::writeDescriptors() start \n");
		for(auto &render_object : render_objects){
			render_object.program->uniformUpdate(
				render_object.descriptor_set,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
				&render_object.ubo->descriptor, nullptr
			);
		}
		LOG("Test::writeDescriptors() end \n");
	}

	void updateUniforms(){
		static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		for(auto &obj : render_objects){
			obj.matices.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));//glm::rotate(obj.matices.model, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			obj.matices.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));//camera.matrices.view;
			obj.matices.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);//camera.matrices.proj;
			obj.matices.proj[1][1] *= -1;
			obj.ubo->copyFrom(&obj.matices, sizeof(obj.matices));
		}
	}

	public:
	void run(){
		Application::init();
		preparePrograms();
		prepareRenderObjects();
		prepareCommandBuffer();
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