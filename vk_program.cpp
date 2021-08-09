#ifndef __VK_PROGRAM_CPP__
#define __VK_PROGRAM_CPP__

#include "vk_program.h"

using namespace std;

namespace VKEngine{
	Program::Program(Context *_context){
		context = _context;
		device = VkDevice(*context);
	}

	void Program::attachShader(const string file_path, VkShaderStageFlagBits stage){
		Shader shader(context, file_path, stage);
		shaders.push_back(shader);
	}

	void Program::build(VkRenderPass render_pass, VkPipelineCache cache){
		vector<VkPipelineShaderStageCreateInfo> stages; 
		for(Shader shader : shaders){
			shader.createShaderModule();
			VkPipelineShaderStageCreateInfo stage_CI = infos::shaderStageCreateInfo("main", shader.module, shader.stage);
			stages.push_back(stage_CI);
		}
		VkPipelineLayoutCreateInfo pipeline_layout_CI = infos::pipelineLayoutCreateInfo(descriptors.layouts.data(), static_cast<uint32_t>(descriptors.layouts.size()));
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipeline_layout_CI, nullptr, &pipeline_layout));

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
		graphics_pipeline_CI.renderPass = render_pass;
		graphics_pipeline_CI.pInputAssemblyState = &input_assembly_state_CI;
		graphics_pipeline_CI.pRasterizationState = &rasterization_state_CI;
		graphics_pipeline_CI.pDepthStencilState = &depth_stencil_state_CI;
		graphics_pipeline_CI.pMultisampleState = &multisample_state_CI;
		graphics_pipeline_CI.pDynamicState = &dynamic_state_CI;
		graphics_pipeline_CI.stageCount = static_cast<uint32_t>(stages.size());
		graphics_pipeline_CI.pStages = stages.data();
		graphics_pipeline_CI.pVertexInputState = &vertex_input_state_CI;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, cache, 1, &graphics_pipeline_CI, nullptr, &pipeline));

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
	
	void Program::createDescriptorPool(vector<VkDescriptorPoolSize> pool_size, uint32_t max_set){
		VkDescriptorPoolCreateInfo pool_info = infos::descriptorPoolCreateInfo(
			static_cast<uint32_t>(pool_size.size()),
			pool_size.data(),
			max_set
		);

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptors.pool));
	}

	void Program::createDescriptorSet(){
		
	}

	void Program::uniformUpdate(){

	}
}
#endif