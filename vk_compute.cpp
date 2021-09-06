#ifndef __VK_COMPUTE_CPP__
#define __VK_COMPUTE_CPP__

#include "vk_compute.h"

using namespace std;

namespace VKEngine{

Kernel::Kernel(){};
Kernel::Kernel(Context *_context, const string _file_path){
	create(_context, _file_path);
}

Kernel::~Kernel(){
	if(layout){
		vkDestroyPipelineLayout(device, layout, nullptr);
		layout = VK_NULL_HANDLE;
	}
	if(pipeline){
		vkDestroyPipeline(device, pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}

	if(descriptors.layout){
		vkDestroyDescriptorSetLayout(device, descriptors.layout, nullptr);
		descriptors.layout = VK_NULL_HANDLE;
	}
}

void Kernel::create(Context *_context, const string _file_path){
	LOG("Kernel create :: file_path : %s\n", _file_path.c_str() );
	context = _context;
	device = VkDevice(*context);
	file_path = _file_path;
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

void Kernel::setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> bindings){
	LOG("Kernel::setupDescriptorSetLayout()\n");
	LOG("bindings size : %d\n", bindings.size());
	VkDescriptorSetLayoutCreateInfo layout_CI =  infos::descriptorSetLayoutCreateInfo(bindings);
	VK_CHECK_RESULT( vkCreateDescriptorSetLayout(device, &layout_CI, nullptr, &descriptors.layout) );
}

/*
void Kernel::build(VkPipelineCache cache){
	LOG("Kernel::build()\n");
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
*/

void Kernel::build(VkPipelineCache cache, VkSpecializationInfo *info){
	LOG("Kernel::build()\n");
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
	shader_stage_CI.pSpecializationInfo = info;
	compute_pipeline_CI.stage = shader_stage_CI;
	VK_CHECK_RESULT( vkCreateComputePipelines(device, cache, 1, &compute_pipeline_CI, nullptr, &pipeline) );

}

void Kernel::setKernelArgs(vector<KernelArgs> args){
	uint32_t sz_args = static_cast<uint32_t>(args.size());
	vector<VkWriteDescriptorSet> writes;
	for(uint32_t i = 0 ; i < sz_args ; ++i){
		VkWriteDescriptorSet write = infos::writeDescriptorSet(
			descriptors.set, args[i].type, args[i].binding_idx, args[i].buffer_info, args[i].image_info
		);
		writes.push_back(write);
	}
	vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}

void Kernel::allocateDescriptorSet(VkDescriptorPool descriptor_pool){
	VkDescriptorSetAllocateInfo info = infos::descriptorSetAllocateInfo(descriptor_pool,
		&descriptors.layout,
		1
	);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &info, &descriptors.set));
}

void Kernel::destroy(){
	vkDestroyDescriptorSetLayout(device, descriptors.layout, nullptr);
	vkDestroyPipelineLayout(device, layout, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);
	destroyShaderModule();
}

};

#endif