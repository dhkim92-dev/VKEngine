#ifndef __VK_SHADER_CPP__
#define __VK_SHADER_CPP__

#include "vk_shader.h"

using namespace std;

namespace VKEngine{
	Shader::Shader(Context *_context, const string file_path, VkShaderStageFlagBits _stage){
		assert(_context);
		context = _context;
		device = VkDevice(*context);
		stage = _stage;
		setShader(file_path);
	}

	Shader::~Shader(){
		destroy();
	}

	void Shader::setShader(const string file_path){
		module = loadShader(file_path, device);
	}

	void Shader::destroy(){
		if(module)
			vkDestroyShaderModule(device, module, nullptr);
		module= VK_NULL_HANDLE;
	}
}


#endif