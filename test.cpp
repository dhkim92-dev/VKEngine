#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

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


struct Camera{
	bool updated = true;
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
};

struct Vertex{
	glm::vec2 pos;
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
		attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributes[0].offset = offsetof(Vertex, pos);

		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].offset = offsetof(Vertex, color);
		return attributes;
	}
};

struct Triangle{
	vector<Vertex> vertices = {
		{{-0.5, 0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f} }
	};

	vector<uint32_t> indices = {
		0,1,2,2,3,0
	};

	Buffer *vbo;
	Buffer *ibo;

	Program *program = nullptr;
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};

class App : public VKEngine::Application{
	public :
	explicit App(string app_name, string engine_name, int h, int w, vector<const char*>instance_exts, vector<const char*>device_exts , vector<const char *>valids) : Application(app_name, engine_name, h, w, instance_exts, device_exts, valids){
	};

	Triangle render_object;
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
		}
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void draw(){
		current_frame_index++;
		current_frame_index%=swapchain.buffers.size();
		render();
	}

	void preparePrograms(){
		LOG("prepare Programs start\n");
		programs.insert({"triangle" , new Program(context) });
		programs["triangle"]->attachShader("./shaders/triangles/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		programs["triangle"]->attachShader("./shaders/triangles/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		programs["triangle"]->setupDescriptorSetLayout({});
		auto attributes = Vertex::vertexInputAttributes();
		auto bindings = Vertex::vertexInputBinding();
		programs["triangle"]->graphics.vertex_input = infos::vertexInputStateCreateInfo(attributes, bindings);
		programs["triangle"]->build(front_framebuffer->render_pass, cache);
		LOG("prepare Programs end\n");
	}

	void prepareRenderObjects(){
		LOG("prepare Render Objects\n");
		render_object.program = programs["triangle"];
		render_object.vbo = new Buffer(
			context,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			sizeof(Vertex) * render_object.vertices.size(),
			nullptr
		);

		render_object.ibo = new Buffer(
			context,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			sizeof(uint16_t) * render_object.indices.size(), nullptr
		);

		graphics_queue->enqueueCopy(render_object.vertices.data(), render_object.vbo,
									0,0, sizeof(Vertex) * render_object.vertices.size());

		graphics_queue->enqueueCopy(render_object.indices.data(), render_object.ibo,
									0,0,sizeof(uint16_t) * render_object.indices.size());

		LOG("prepare Render Objects end\n");
	}

	void prepareCommandBuffer(){
		vector<VkClearValue> clear_values(2);
		clear_values[0].color = {0.0f, 0.0f, 0.2f ,1.0f};
		clear_values[1].depthStencil = {1.0f, 0};
		draw_command_buffers.resize(swapchain.buffers.size());
		
		VkRenderPassBeginInfo render_pass_BI = infos::renderPassBeginInfo();
		render_pass_BI.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_BI.pClearValues = clear_values.data();
		render_pass_BI.renderArea.offset = {0,0};
		render_pass_BI.renderArea.extent.height = height;
		render_pass_BI.renderArea.extent.width = width;
		render_pass_BI.renderPass = front_framebuffer->render_pass;

		for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
			draw_command_buffers[i] = graphics_queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}

		for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
			graphics_queue->beginCommandBuffer(draw_command_buffers[i]);
			render_pass_BI.framebuffer = front_framebuffer->framebuffers[i];
			VkViewport viewport = infos::viewport(static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
			VkRect2D scissor = infos::rect2D(width, height, 0, 0);
			vkCmdBeginRenderPass(draw_command_buffers[i], &render_pass_BI, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(draw_command_buffers[i], 0, 1, &viewport);
			vkCmdSetScissor(draw_command_buffers[i], 0, 1, &scissor);
			vkCmdBindPipeline(draw_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, render_object.program->pipeline);
			VkBuffer buffer[] = {VkBuffer(*render_object.vbo)};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(draw_command_buffers[i], 0, 1, buffer, offsets);
			vkCmdBindIndexBuffer(draw_command_buffers[i], VkBuffer(*render_object.ibo), 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(draw_command_buffers[i], static_cast<uint32_t>(render_object.indices.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(draw_command_buffers[i]);
			graphics_queue->endCommandBuffer(draw_command_buffers[i]);
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