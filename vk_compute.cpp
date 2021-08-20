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

void Kernel::build(){
	VkPipelineLayoutCreateInfo layout_CI = 
}



};

#endif