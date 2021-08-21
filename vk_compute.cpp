#ifndef __VK_COMPUTE_CPP__
#define __VK_COMPUTE_CPP__

#include "vk_compute.h"

using namespace std;

namespace VKEngine{

Kernel::Kernel(){};
Kernel::Kernel(Context *_context, const string _file_path){
	create(_context, file_path);
}

void Kernel::create(Context *_context, const string _file_path){
	loadShaderModule();
}

void Kernel::loadShaderModule(){
	assert(context != nullptr);
	assert(device != VK_NULL_HANDLE);
	module = loadShader(file_path, device);
}

void Kernel::destroyShaderModule(){
	if(module){
		vkDestroyShaderModule(device, module, nullptr);
		module = VK_NULL_HANDLE;
	}
}

void Kernel::setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> &bindings){
	VkDescriptorSetLayoutCreateInfo layout_CI =  infos::descriptorSetLayoutCreateInfo(bindings);
	VK_CHECK_RESULT( vkCreateDescriptorSetLayout(device, &layout_CI, nullptr, &descriptors.layout) );
}

void Kernel::build(VkPipelineCache cache){
	VkPipelineLayoutCreateInfo layout_CI = infos::pipelineLayoutCreateInfo(
		&descriptors.layout, 1
	);
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &layout_CI, nullptr, &layout));
	VkComputePipelineCreateInfo compute_pipeline_CI = infos::computePipelineCreateInfo(layout);
	VkPipelineShaderStageCreateInfo shader_stage_CI = infos::shaderStageCreateInfo(
		"main",
		module,
		VK_SHADER_STAGE_COMPUTE_BIT
	);
	compute_pipeline_CI.stage = shader_stage_CI;
	VK_CHECK_RESULT( vkCreateComputePipelines(device, cache, 1, &compute_pipeline_CI, nullptr, &pipeline) );
}
};

#endif