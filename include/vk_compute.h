#ifndef __VK_COMPUTE_H__
#define __VK_COMPUTE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "vk_context.h"
#include "vk_pipeline.h"
#include "vk_utils.h"
#include "vk_infos.h"

using namespace std;

namespace VKEngine{

class ComputePipelineBuilder{
	private :
	Context *context = nullptr;
	VkShaderModule compute_shader = VK_NULL_HANDLE;
	VkPipelineShaderStageCreateInfo stage;
	public :
	explicit ComputePipelineBuilder(Context *ctx);
	~ComputePipelineBuilder();
	void setComputeShader(string file_path, VkSpecializationInfo *sinfo = nullptr);
	VkResult build(VkPipeline *pipeline, VkPipelineLayout layout, VkPipelineCache cache, VkPipelineCreateFlags flags=0);
	void destroy(VkPipeline *pipeline);
	static void destroy(Context *ctx, VkPipeline *pipeline);
};
}
#endif