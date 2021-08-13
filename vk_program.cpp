#ifndef __VK_PROGRAM_CPP__
#define __VK_PROGRAM_CPP__

#include "vk_program.h"

using namespace std;

namespace VKEngine{
	Program::Program(Context *_context){
		context = _context;
		device = VkDevice(*context);
	}

	Program::~Program(){
		destroy();
	}

	void Program::attachShader(const string file_path, VkShaderStageFlagBits stage){
		Shader shader(context, file_path, stage);
		shaders.push_back(shader);
	}

	void Program::build(VkRenderPass render_pass, VkPipelineCache cache){
		LOG("Program::build start\n");
		vector<VkPipelineShaderStageCreateInfo> stages; 
		for(Shader shader : shaders){
			shader.createShaderModule();
			VkPipelineShaderStageCreateInfo stage_CI = infos::shaderStageCreateInfo("main", shader.module, shader.stage);
			LOG("Program::build stage_CI added : %p %d\n", shader.module, shader.stage);
			stages.push_back(stage_CI);
		}
		VkPipelineLayoutCreateInfo pipeline_layout_CI = infos::pipelineLayoutCreateInfo(descriptors.layouts.data(), static_cast<uint32_t>(descriptors.layouts.size()));
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipeline_layout_CI, nullptr, &pipeline_layout));
		LOG("Program::build done create pipeline layout done\n");

		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_CI = infos::inputAssemblyStateCreateInfo(input_assembly_state.topology,input_assembly_state.flags,input_assembly_state.primitive_restart);
		VkPipelineRasterizationStateCreateInfo rasterization_state_CI = infos::rasterizationStateCreateInfo(rasterization_state.polygon,rasterization_state.cull,rasterization_state.face,rasterization_state.flags);
		VkPipelineColorBlendAttachmentState color_blend_attachment = infos::colorBlendAttachmentState(color_blend_attachment_state.color_write_mask,color_blend_attachment_state.blend_enable);
		VkPipelineColorBlendStateCreateInfo color_blend_state_CI = infos::colorBlendStateCreateInfo(1, &color_blend_attachment);
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_CI = infos::depthStencilStateCreateInfo(depth_stencil_state.depth_test_enabled,depth_stencil_state.depth_write_enabled,depth_stencil_state.depth_compare_OP);
		VkPipelineViewportStateCreateInfo viewport_state_CI = infos::viewportStateCreateInfo(viewport_state.nr_viewports, viewport_state.nr_scissors, viewport_state.flags);
		VkPipelineMultisampleStateCreateInfo multisample_state_CI = infos::multisampleStateCreateInfo(multisample_state.rasterization_samples, multisample_state.flags);
		VkPipelineDynamicStateCreateInfo dynamic_state_CI = infos::dynamicStateCreateInfo(dynamic_state_enabled);
		VkPipelineVertexInputStateCreateInfo vertex_input_state_CI = infos::vertexInputStateCreateInfo(vertex_input_state.attributes, vertex_input_state.bindings);
		VkGraphicsPipelineCreateInfo graphics_pipeline_CI = infos::graphicsPipelineCreateInfo(pipeline_layout, render_pass);
		graphics_pipeline_CI.pStages = stages.data();
		graphics_pipeline_CI.stageCount = static_cast<uint32_t>(stages.size());
		graphics_pipeline_CI.renderPass = render_pass;
		graphics_pipeline_CI.pVertexInputState = &vertex_input_state_CI;
		graphics_pipeline_CI.pInputAssemblyState = &input_assembly_state_CI;
		graphics_pipeline_CI.pViewportState = &viewport_state_CI;
		graphics_pipeline_CI.pRasterizationState = &rasterization_state_CI;
		graphics_pipeline_CI.pMultisampleState = &multisample_state_CI;
		graphics_pipeline_CI.pColorBlendState = &color_blend_state_CI;
		graphics_pipeline_CI.pDepthStencilState = &depth_stencil_state_CI;
		graphics_pipeline_CI.pDynamicState = &dynamic_state_CI;
		graphics_pipeline_CI.subpass = 0;
		graphics_pipeline_CI.basePipelineHandle = VK_NULL_HANDLE;




		LOG("Program::build graphics pipeline\n");
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, cache, 1, &graphics_pipeline_CI, nullptr, &pipeline));
		LOG("Program::build graphics pipeline done\n");

		for(Shader shader : shaders){
			shader.destroy();
		}
	}

	void Program::setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> bindings){
		if(bindings.size() > 0){
			descriptors.bindings.resize(bindings.size());
			descriptors.bindings.assign( bindings.begin(), bindings.end() );
		}
		VkDescriptorSetLayoutCreateInfo layout_CI = infos::descriptorSetLayoutCreateInfo(descriptors.bindings);
		VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layout_CI, nullptr, &descriptor_set_layout));
		descriptors.layouts.push_back(descriptor_set_layout);
	}
	
	VkResult Program::createDescriptorPool(vector<VkDescriptorPoolSize> pool_size, uint32_t max_set){
		VkDescriptorPoolCreateInfo pool_CI = infos::descriptorPoolCreateInfo( static_cast<uint32_t>(pool_size.size()), pool_size.data(), max_set);
		return vkCreateDescriptorPool(device, &pool_CI, nullptr, &descriptors.pool);
	}

	VkResult Program::allocDescriptorSet(VkDescriptorSet *descriptor_set, uint32_t set_idx, uint32_t nr_alloc){
		assert(descriptor_pool != VK_NULL_HANDLE);
		VkDescriptorSetAllocateInfo descriptor_AI = infos::descriptorSetAllocateInfo(descriptors.pool, &descriptors.layouts[set_idx], nr_alloc);
		return vkAllocateDescriptorSets(device, &descriptor_AI, descriptor_set);
	}

	void Program::releaseDescriptorSet(VkDescriptorSet *descriptor_set){
		vkFreeDescriptorSets(device, descriptors.pool, 1, descriptor_set);
	}

	void Program::uniformUpdate( VkDescriptorSet descriptor_set, VkDescriptorType type, uint32_t binding_idx, VkDescriptorBufferInfo *buffer_info, VkDescriptorImageInfo *image_info){
		VkWriteDescriptorSet write_info = infos::writeDescriptorSet( descriptor_set, type, binding_idx, buffer_info, image_info);
		vkUpdateDescriptorSets(device, 1, &write_info, 0, nullptr);
	}

	void Program::destroy(){
		if(pipeline) vkDestroyPipeline(device, pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
		if(pipeline_layout) vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
		pipeline_layout = VK_NULL_HANDLE;
		if(descriptors.pool) vkDestroyDescriptorPool(device, descriptors.pool, nullptr);
		descriptors.pool = VK_NULL_HANDLE;
		for(VkDescriptorSetLayout &layout : descriptors.layouts){
			if(layout) vkDestroyDescriptorSetLayout(device, layout, nullptr);
			layout = VK_NULL_HANDLE;
		}
	}
}
#endif