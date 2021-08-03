#ifndef __VK_PROGRAM_H__
#define __VK_PROGRAM_H__

#include <vector>
#include <string>
#include "vk_context.h"
#include "vk_shader.h"
#include "vk_utils.h"
#include "vk_infos.h"

using namespace std;

namespace VKEngine{
	class Program{
		public :
		VkPipeline pipeline = VK_NULL_HANDLE;
		private :
		Context *context = nullptr;
		VkDevice device = VK_NULL_HANDLE;
		vector<Shader> shaders;
		
		public :
		explicit Program(Context *_context);
		~Program();
		void attachShader(const string file_path, VkShaderStageFlagBits stage);
		void build(VkRenderPass render_pass, VkPipelineCache cache);
		void updateUniform();
	};
}

#endif