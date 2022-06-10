#ifndef __VK_PIPELINE_CPP__
#define __VK_PIPELINE_CPP__

#include "vk_pipeline.h"

namespace VKEngine{

// PipelineLayoutBuilder
VkResult PipelineLayoutBuilder::build(
	Context *ctx, VkPipelineLayout* dst,
	VkDescriptorSetLayout *layouts, uint32_t layouts_count,
	VkPushConstantRange *push_constants_ranges, uint32_t push_constants_count,
	VkPipelineLayoutCreateFlags flags, void *pNext)
{
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pSetLayouts = layouts;
	info.setLayoutCount = layouts_count;
	info.pPushConstantRanges = push_constants_ranges;
	info.pushConstantRangeCount = push_constants_count;
	info.pNext = pNext;

	return vkCreatePipelineLayout(ctx->getDevice(), &info, nullptr, dst);
}

void PipelineLayoutBuilder::destroy(Context *ctx, VkPipelineLayout *layout)
{
	if(*layout != VK_NULL_HANDLE){
		vkDestroyPipelineLayout(ctx->getDevice(), *layout, nullptr);
		*layout = VK_NULL_HANDLE;
	}
}

// PipelineCacheBuilder
VkResult PipelineCacheBuilder::build(Context *ctx, VkPipelineCache *cache, size_t init_data_size, void * p_init_data)
{
	VkPipelineCacheCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	info.pNext = nullptr;
	info.initialDataSize = init_data_size;
	info.pInitialData = p_init_data;
	return vkCreatePipelineCache(ctx->getDevice(), &info, nullptr, cache);
}

void PipelineCacheBuilder::destroy(Context *ctx, VkPipelineCache *cache)
{
	if(*cache != VK_NULL_HANDLE){
		vkDestroyPipelineCache(ctx->getDevice(), *cache, nullptr);
		*cache = VK_NULL_HANDLE;
	}
}

//Graphics Pipeline Builder
GraphicsPipelineBuilder::GraphicsPipelineBuilder(Context *context) : context(context){
	vertex_shader = VK_NULL_HANDLE;
	fragment_shader = VK_NULL_HANDLE;
	geometry_shader = VK_NULL_HANDLE;
	tesselation_control_shader = VK_NULL_HANDLE;
	tesselation_evaluation_shader= VK_NULL_HANDLE;
};

void GraphicsPipelineBuilder::createVertexShader(const string file_path)
{
	LOG("create Vertex Shader called\n");
	// if(vertex_shader != VK_NULL_HANDLE) return ;
	vertex_shader = loadShader(file_path, context->getDevice());
	LOG("vertex shader generate\n");
	VkPipelineShaderStageCreateInfo info = infos::shaderStageCreateInfo("main", vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
	stages.push_back(info);
	LOG("stage size : %d\n",stages.size());

}

void GraphicsPipelineBuilder::createFragmentShader(const string file_path)
{
	LOG("createFragmentShader called\n");
	// if(fragment_shader != VK_NULL_HANDLE) return;
	fragment_shader = loadShader(file_path, context->getDevice());
	LOG("frag shader generate\n");
	VkPipelineShaderStageCreateInfo info = infos::shaderStageCreateInfo("main", fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);
	stages.push_back(info);
	LOG("stage size : %d\n",stages.size());
}

void GraphicsPipelineBuilder::createGeometryShader(const string file_path)
{
	if(geometry_shader != VK_NULL_HANDLE) throw std::runtime_error("Fragment Shader already generated.\n");

	VkPhysicalDeviceFeatures features = context->getPhysicalDevice()->getDeviceFeatures();
	if(features.geometryShader == VK_TRUE){
		geometry_shader = loadShader(file_path, context->getDevice());
		if(geometry_shader == VK_NULL_HANDLE){
			VkPipelineShaderStageCreateInfo info = infos::shaderStageCreateInfo("main", geometry_shader, VK_SHADER_STAGE_GEOMETRY_BIT);
			stages.push_back(info);
			LOG("stage size : %d\n",stages.size());
		}
	}else{
		std::runtime_error("Your GPU does not support geometry shader.\n");
	}
}

void GraphicsPipelineBuilder::createTesselationControlShader(const string file_path)
{
	if(tesselation_control_shader != VK_NULL_HANDLE) throw std::runtime_error("Fragment Shader already generated.\n");

	VkPhysicalDeviceFeatures features = context->getPhysicalDevice()->getDeviceFeatures();
	if(features.tessellationShader == VK_TRUE){
		tesselation_control_shader= loadShader(file_path, context->getDevice());
		if(tesselation_control_shader == VK_NULL_HANDLE){
			VkPipelineShaderStageCreateInfo info = infos::shaderStageCreateInfo("main", tesselation_control_shader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
			stages.push_back(info);
		}
	}else{
		std::runtime_error("Your GPU does not support tesselation shader.\n");
	}
}

void GraphicsPipelineBuilder::createTesselationEvaluationShader(const string file_path)
{
	if(tesselation_evaluation_shader != VK_NULL_HANDLE) throw std::runtime_error("Fragment Shader already generated.\n");

	VkPhysicalDeviceFeatures features = context->getPhysicalDevice()->getDeviceFeatures();
	if(features.tessellationShader == VK_TRUE){
		tesselation_evaluation_shader= loadShader(file_path, context->getDevice());
		if(tesselation_evaluation_shader != VK_NULL_HANDLE){
			VkPipelineShaderStageCreateInfo info = infos::shaderStageCreateInfo("main", tesselation_evaluation_shader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
			stages.push_back(info);
		}
	}else{
		std::runtime_error("Your GPU does not support tesselation shader.\n");
	}
}

void GraphicsPipelineBuilder::setVertexInputState(VkPipelineVertexInputStateCreateInfo info)
{
	states.vertex = info;
}


VkResult GraphicsPipelineBuilder::build(VkPipeline *pipeline, VkRenderPass renderpass, VkPipelineLayout layout, VkPipelineCache cache, uint32_t subpass)
{
	VkGraphicsPipelineCreateInfo info={};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.renderPass = renderpass;
	info.layout = layout;
	info.stageCount = stages.size();
	info.pStages = stages.data();
	LOG("stage size : %d\n",stages.size());
	info.pInputAssemblyState = &states.input_assembly;
	info.pColorBlendState = &states.color_blend;
	info.pRasterizationState = &states.rasterization;
	info.pDynamicState = &states.dynamic;
	info.pDepthStencilState = &states.depth_stencil;
	info.pMultisampleState = &states.multi_sample;
	info.pViewportState = &states.viewport;
	info.pTessellationState = &states.tesselation;
	info.pVertexInputState = &states.vertex;
	info.subpass=subpass;

	return vkCreateGraphicsPipelines(context->getDevice(), cache, 1, &info, nullptr, pipeline);
}

void GraphicsPipelineBuilder::destroy(VkPipeline *pipeline)
{
	if(*pipeline != VK_NULL_HANDLE){
		vkDestroyPipeline(context->getDevice(), *pipeline, nullptr);
		*pipeline=VK_NULL_HANDLE;
	}
}

void GraphicsPipelineBuilder::destroy(Context *ctx, VkPipeline *pipeline)
{
	if(*pipeline !=VK_NULL_HANDLE){
		vkDestroyPipeline(ctx->getDevice(), *pipeline, nullptr);
		*pipeline = VK_NULL_HANDLE;
	}
}

void GraphicsPipelineBuilder::setInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo info)
{
	states.input_assembly = info;
}

void GraphicsPipelineBuilder::setRasterizationState(VkPipelineRasterizationStateCreateInfo info)
{
	states.rasterization = info;
}

void GraphicsPipelineBuilder::setColorBlendState(VkPipelineColorBlendStateCreateInfo info)
{
	states.color_blend = info;
}

void GraphicsPipelineBuilder::setDepthStencilState(VkPipelineDepthStencilStateCreateInfo info)
{
	states.depth_stencil = info;
}

void GraphicsPipelineBuilder::setMultiSampleState(VkPipelineMultisampleStateCreateInfo info)
{
	states.multi_sample=info;
}

void GraphicsPipelineBuilder::setViewportState(VkPipelineViewportStateCreateInfo info)
{
	states.viewport = info;
}

void GraphicsPipelineBuilder::setDynamicState(VkPipelineDynamicStateCreateInfo info)
{
	states.dynamic = info;
}

void GraphicsPipelineBuilder::setInputAssemblyState(VkPrimitiveTopology topology, VkBool32 restart_enable, VkPipelineInputAssemblyStateCreateFlags flags, void *pNext)
{
	VkPipelineInputAssemblyStateCreateInfo info={};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.primitiveRestartEnable = restart_enable;
	info.pNext = pNext;
	info.flags = flags;
	info.topology = topology;
	setInputAssemblyState(info);
}

void GraphicsPipelineBuilder::setRasterizationState(
	VkPolygonMode mode, VkCullModeFlags cull, VkFrontFace face, 
	VkPipelineRasterizationStateCreateFlags flags, 
	void *pNext, 
	VkBool32 raster_discard_enable,
	VkBool32 depth_clamp_enable,
	VkBool32 depth_bias_enable,
	float depth_bias_constant_factor, float depth_bias_clamp, 
	float depth_bias_slope_factor, float line_width)
{
	VkPipelineRasterizationStateCreateInfo info={};
	info.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	info.cullMode = cull;
	info.polygonMode = mode;
	info.frontFace = face;
	info.flags = flags;
	info.pNext = pNext;
	info.rasterizerDiscardEnable = raster_discard_enable;
	info.depthClampEnable = depth_clamp_enable;
	info.depthBiasEnable = depth_bias_enable;
	info.depthBiasSlopeFactor = depth_bias_slope_factor;
	info.depthBiasClamp = depth_bias_clamp;
	info.depthBiasConstantFactor = depth_bias_constant_factor;
	info.lineWidth = line_width;
	setRasterizationState(info);
}

void GraphicsPipelineBuilder::setViewportState(uint32_t nr_viewports, VkViewport *p_viewports, uint32_t nr_scissors, VkRect2D *p_scissors, VkPipelineViewportStateCreateFlags flags, void *pNext){
	VkPipelineViewportStateCreateInfo info={};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	info.flags = flags;
	info.viewportCount = nr_viewports;
	info.pViewports = p_viewports;
	info.scissorCount = nr_scissors;
	info.pScissors = p_scissors;
	info.pNext = pNext;

	setViewportState(info);
}

void GraphicsPipelineBuilder::setDynamicState(uint32_t nr_dstates, VkDynamicState *p_dynamics, VkPipelineDynamicStateCreateFlags flags, void *pNext)
{
	VkPipelineDynamicStateCreateInfo info={};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	info.flags = flags;
	info.pDynamicStates = p_dynamics;
	info.dynamicStateCount = nr_dstates;
	info.pNext = pNext;
	setDynamicState(info);
}


GraphicsPipelineBuilder::~GraphicsPipelineBuilder()
{
	VkDevice device = context->getDevice();
	if(vertex_shader != VK_NULL_HANDLE){
		vkDestroyShaderModule(device, vertex_shader, nullptr);
		vertex_shader = VK_NULL_HANDLE;
	}

	if(fragment_shader != VK_NULL_HANDLE){
		vkDestroyShaderModule(device, fragment_shader, nullptr);
		fragment_shader = VK_NULL_HANDLE;
	}

	if(geometry_shader != VK_NULL_HANDLE){
		vkDestroyShaderModule(device, geometry_shader, nullptr);
		geometry_shader = VK_NULL_HANDLE;
	}
	
	if(tesselation_control_shader != VK_NULL_HANDLE){
		vkDestroyShaderModule(device, tesselation_control_shader, nullptr);
		tesselation_control_shader = VK_NULL_HANDLE;
	}
	if(tesselation_evaluation_shader != VK_NULL_HANDLE){
		vkDestroyShaderModule(device, tesselation_evaluation_shader, nullptr);
		tesselation_evaluation_shader = VK_NULL_HANDLE;
	}
}


}
#endif