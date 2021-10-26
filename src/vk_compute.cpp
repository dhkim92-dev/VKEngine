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
	destroy();
}

void Kernel::create(Context *_context, const string _file_path){
	//LOG("Kernel create :: file_path : %s\n", _file_path.c_str() );
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

VkResult Kernel::setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> bindings){
	//LOG("Kernel::setupDescriptorSetLayout()\n");
	//LOG("bindings size : %d\n", bindings.size());
	VkDescriptorSetLayoutCreateInfo layout_CI =  infos::descriptorSetLayoutCreateInfo(bindings);
	return vkCreateDescriptorSetLayout(device, &layout_CI, nullptr, &layouts.descriptor);
}

void Kernel::build(VkPipelineCache cache, VkSpecializationInfo *info){
	//LOG("Kernel::build()\n");
	VkPipelineLayoutCreateInfo layout_CI = infos::pipelineLayoutCreateInfo(
		&layouts.descriptor, 1
	);

	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &layout_CI, nullptr, &layouts.pipeline));
	VkComputePipelineCreateInfo compute_pipeline_CI = infos::computePipelineCreateInfo(layouts.pipeline);
	VkPipelineShaderStageCreateInfo shader_stage_CI = infos::shaderStageCreateInfo(
		"main",
		module,
		VK_SHADER_STAGE_COMPUTE_BIT
	);
	shader_stage_CI.pSpecializationInfo = info;
	compute_pipeline_CI.stage = shader_stage_CI;
	VK_CHECK_RESULT( vkCreateComputePipelines(device, cache, 1, &compute_pipeline_CI, nullptr, &pipeline) );

}

void Kernel::setKernelArgs(VkDescriptorSet set, vector<KernelArgs> args){
	uint32_t sz_args = static_cast<uint32_t>(args.size());
	vector<VkWriteDescriptorSet> writes;
	for(uint32_t i = 0 ; i < sz_args ; ++i){
		VkWriteDescriptorSet write = infos::writeDescriptorSet(
			set, args[i].type, args[i].binding_idx, args[i].buffer_info, args[i].image_info
		);
		writes.push_back(write);
	}
	vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}

VkResult Kernel::allocateDescriptorSet(VkDescriptorPool descriptor_pool, VkDescriptorSet *set, uint32_t nr_descriptor_set){
	VkDescriptorSetAllocateInfo info = infos::descriptorSetAllocateInfo(descriptor_pool,
		&layouts.descriptor,
		nr_descriptor_set
	);
	return vkAllocateDescriptorSets(device, &info, set);
}

void Kernel::destroy(){
	if(layouts.pipeline){
		vkDestroyPipelineLayout(device, layouts.pipeline, nullptr);
		layouts.pipeline = VK_NULL_HANDLE;
	}
	if(pipeline){
		vkDestroyPipeline(device, pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}

	if(layouts.descriptor){
		vkDestroyDescriptorSetLayout(device, layouts.descriptor, nullptr);
		layouts.descriptor = VK_NULL_HANDLE;
	}
	destroyShaderModule();
}

};

#endif
