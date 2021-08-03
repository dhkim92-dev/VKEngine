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
		VkGraphicsPipelineCreateInfo graphics_pipeline_CI = infos::graphicsPipelineCreateInfo(render_pass);
		vector<VkPipelineShaderStageCreateInfo> stages; 

		for(Shader shader : shaders){
			VkPipelineShaderStageCreateInfo stage_CI = infos::shaderStageCreateInfo(
				"main",
				shader.module,
				shader.stage
			);
		}

		graphics_pipeline_CI.pStages = stages.data();
		graphics_pipeline_CI.stageCount = static_cast<uint32_t>(stages.size());
		graphics_pipeline_CI.renderPass = render_pass;
	}
}

#endif