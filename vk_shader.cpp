#ifndef __VK_SHADER_CPP__
#define __VK_SHADER_CPP__

#include "vk_shader.h"

using namespace std;

namespace VKEngine{
	Shader::Shader(Context *_context, const string _file_path, VkShaderStageFlagBits _stage){
		assert(_context);
		context = _context;
		device = VkDevice(*context);
		file_path = _file_path;
		stage = _stage;
	}

	Shader::~Shader(){
		destroy();
	}

	void Shader::createShaderModule(){
		LOG("Shader file_path : %s build, current module : %p\n", file_path.c_str(), module);
		module = loadShader(file_path, device);
		LOG("Shader file_path : %s build, current module : %p\n", file_path.c_str(), module);
	}

	void Shader::destroy(){
		if(module)
			vkDestroyShaderModule(device, module, nullptr);
		module= VK_NULL_HANDLE;
	}
}


#endif