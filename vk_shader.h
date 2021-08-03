#ifndef __VK_SHADER_H__
#define __VK_SHADER_H__

#include <vulkan/vulkan.h>
#include <string>
#include "vk_context.h"
#include "vk_utils.h"

namespace VKEngine{

struct Shader{
	private : 
	Context *context = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	public :
	VkShaderModule module = VK_NULL_HANDLE;
	VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL_GRAPHICS;

	public:
	explicit Shader(Context *context, const string file_path, VkShaderStageFlagBits _stage);
	void setShader(const string file_path);
	void destroy();
};

}
#endif