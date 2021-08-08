#ifndef __VK_APPLICATION_CPP__
#define __VK_APPLICATION_CPP__

#include "vk_application.h"

using namespace std;

namespace VKEngine{
	Application::Application(
		string app_name, string engine_name,
		uint32_t _height, uint32_t _width, 
		const vector<const char *> _instance_extension_names,
		const vector<const char *> _device_extension_names,
		const vector<const char *> _validation_names)
	{
		LOG("application create called!\n");
		instance_extension_names.resize(_instance_extension_names.size());
		device_extension_names.resize(_device_extension_names.size());
		validation_names.resize(_validation_names.size());
		instance_extension_names.assign(_instance_extension_names.begin(), _instance_extension_names.end());
		device_extension_names.assign(_device_extension_names.begin(), _device_extension_names.end());
		validation_names.assign(_validation_names.begin(), _validation_names.end());
		height = _height;
		width = _width;
		engine = new Engine(app_name, engine_name, instance_extension_names, validation_names);
	}	

	Application::~Application(){
		destroy();
	}

	void Application::initVulkan(){
		engine->init();
		LOG("engine init\n");
		VkInstance instance = VkInstance(*engine);
		createSurface();
		context = new Context(instance, 0, surface, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT, device_extension_names, validation_names);
		swapchain.connect(engine, context, surface);
		swapchain.create(&height, &width, false);
		setupCommandQueue();
		setupPipelineCache();
		front_framebuffer = new Framebuffer(context);
		front_framebuffer->height = height;
		front_framebuffer->width = width;
		setupColorAttachment();
		setupDepthStencilAttachment();
		setupRenderPass();
		setupFramebuffer();

	}


	void Application::setupCommandQueue(){
		LOG("Apllication::setupCommandQueue called\n");
		graphics_queue = new CommandQueue(context, VK_QUEUE_GRAPHICS_BIT);
		compute_queue = new CommandQueue(context, VK_QUEUE_COMPUTE_BIT);
	}
	
	void Application::init(){
		initWindow();
		initVulkan();
	}	

	void Application::setupPipelineCache(){
		VkPipelineCacheCreateInfo cache_CI = {};
		cache_CI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT( vkCreatePipelineCache(VkDevice(*context), &cache_CI, nullptr, &cache) );
	}

	void Application::setupDepthStencilAttachment(){
		LOG("Application::setupDepthStencilAttachment\n");
		AttachmentCreateInfo info;
		VkBool32 found = getDepthFormat( VkPhysicalDevice(*context), &depth_format);
		info.height = height;
		info.width = width;
		info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		info.sample_count = VK_SAMPLE_COUNT_1_BIT;
		info.nr_layers = 1;
		info.format = depth_format;
		front_framebuffer->addAttachment(info);
		LOG("end Application::setupDepthStencilAttachment\n");
	}

	void Application::setupColorAttachment(){
		LOG("Application::setupColorAttachment\n");
		FramebufferAttachment attachment = {};
		attachment.image = swapchain.buffers[0].image;
		attachment.view = swapchain.buffers[0].view;
		attachment.format = swapchain.image_format;
		attachment.subresource_range = {1, 0, 1, 0};
		attachment.description.format = swapchain.image_format;
		attachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		front_framebuffer->attachments.push_back(attachment);
		LOG("end Application::setupColorAttachment\n");
	}

	void Application::setupRenderPass(){
		LOG("Application::setupRenderPass\n");
		front_framebuffer->createRenderPass();
		LOG("end Application::setupRenderPass\n");
	}

	void Application::setupFramebuffer(){
		LOG("Application::setupFramebuffer\n");
		front_framebuffer->createFramebuffer();
		LOG("end Application::setupFramebuffer\n");
	}

	void Application::setupGraphicsPipeline(){
		graphics_pipeline_CI_preset = infos::graphicsPipelineCreateInfo(front_framebuffer->render_pass);
		VkPipelineInputAssemblyStateCreateInfo input_assembly_CI = infos::inputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterization_CI = infos::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState blend_attachment_state = infos::colorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo blend_state_CI = infos::colorBlendStateCreateInfo(1, &blend_attachment_state);
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_CI = infos::depthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewport_state_CI = infos::viewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisample_CI = infos::multisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		vector<VkDynamicState> dynamic_state_enabled = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic_state_CI = infos::dynamicStateCreateInfo( dynamic_state_enabled );
				
	}
	
	void Application::destroy(){
		delete graphics_queue;
		delete compute_queue;
		swapchain.destroy();
		delete front_framebuffer;
		delete context;
		delete engine;
	}
}
#endif