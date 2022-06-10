#ifndef __VK_COMPUTE_CPP__
#define __VK_COMPUTE_CPP__

#include "vk_compute.h"

using namespace std;

namespace VKEngine{

ComputePipelineBuilder::ComputePipelineBuilder(Context *ctx) : context(ctx){};
ComputePipelineBuilder::~ComputePipelineBuilder(){
	if(compute_shader != VK_NULL_HANDLE){
		vkDestroyShaderModule(context->getDevice(), compute_shader, nullptr);
		compute_shader = VK_NULL_HANDLE;
	}
}

void ComputePipelineBuilder::setComputeShader(string file_path, VkSpecializationInfo *sinfo)
{
	if(compute_shader==VK_NULL_HANDLE)
		compute_shader = loadShader(file_path, context->getDevice());

	VkPipelineShaderStageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	info.pName = "main";
	info.pNext = nullptr;
	info.module = compute_shader;
	info.pSpecializationInfo = sinfo;
	stage = info;
}

VkResult ComputePipelineBuilder::build(VkPipeline *pipeline, VkPipelineLayout layout, VkPipelineCache cache, VkPipelineCreateFlags flags)
{
	VkComputePipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	info.layout = layout;
	info.stage = stage;
	info.pNext = nullptr;
	info.flags = flags;
	return vkCreateComputePipelines(context->getDevice(), cache, 1, &info, nullptr, pipeline);
}

void ComputePipelineBuilder::destroy(VkPipeline *pipeline)
{
	if(*pipeline != VK_NULL_HANDLE){
		vkDestroyPipeline(context->getDevice(), *pipeline, nullptr);
		*pipeline = VK_NULL_HANDLE;
	}
}
void ComputePipelineBuilder::destroy(Context *ctx, VkPipeline *pipeline)
{
	if(*pipeline != VK_NULL_HANDLE){
		vkDestroyPipeline(ctx->getDevice(), *pipeline, nullptr);
		*pipeline = VK_NULL_HANDLE;
	}
}



};

#endif
