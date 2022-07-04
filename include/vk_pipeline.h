#ifndef __VK_PIPELINE_H__
#define __VK_PIPELINE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "vk_context.h"
#include "vk_utils.h"
#include "vk_infos.h"

using namespace std;

namespace VKEngine{

class PipelineCacheBuilder
{
	public :
	static VkResult build(Context *ctx, VkPipelineCache *cache, size_t init_data_size=0, void *pInitData=nullptr);
	static void destroy(Context *ctx, VkPipelineCache *cache);
};

class PipelineLayoutBuilder
{
	public:
	static VkResult build(Context *ctx, VkPipelineLayout* dst,
		 VkDescriptorSetLayout *layouts, uint32_t layouts_count, 
		 VkPushConstantRange *push_constant_ranges=nullptr, uint32_t push_constant_count=0, 
		 VkPipelineLayoutCreateFlags flags=0, void *pNext = nullptr);
		
	static void destroy(Context *ctx, VkPipelineLayout *layout);
};

struct GraphicsPipelineStatesCreateInfos{
	VkPipelineInputAssemblyStateCreateInfo input_assembly={};
	VkPipelineRasterizationStateCreateInfo rasterization={};
	VkPipelineColorBlendStateCreateInfo color_blend={};
	VkPipelineDepthStencilStateCreateInfo depth_stencil={};
	VkPipelineMultisampleStateCreateInfo multi_sample={};
	VkPipelineViewportStateCreateInfo viewport={};
	VkPipelineDynamicStateCreateInfo dynamic={};
	VkPipelineVertexInputStateCreateInfo vertex={};
	VkPipelineTessellationStateCreateInfo tesselation={};
};

class GraphicsPipelineBuilder
{	
	//fields
	private:
	Context *context=nullptr;
	VkShaderModule vertex_shader = VK_NULL_HANDLE;
	VkShaderModule fragment_shader = VK_NULL_HANDLE;
	VkShaderModule geometry_shader = VK_NULL_HANDLE;
	VkShaderModule tesselation_control_shader = VK_NULL_HANDLE;
	VkShaderModule tesselation_evaluation_shader = VK_NULL_HANDLE;
	vector<VkPipelineShaderStageCreateInfo> stages;
	GraphicsPipelineStatesCreateInfos states;
	VkPipelineCreateFlags pipeline_flags;

	//methods
	private:
	public:
	GraphicsPipelineBuilder(Context *ctx);
	~GraphicsPipelineBuilder();
	void createVertexShader(const string path);
	void createFragmentShader(const string path);
	void createTesselationControlShader(const string path);
	void createTesselationEvaluationShader(const string path);
	void createGeometryShader(const string path);
	VkResult build(VkPipeline* pipeline, VkRenderPass renderpass, VkPipelineLayout layout, VkPipelineCache cache, uint32_t subpass=0);
	static void destroy(Context *ctx, VkPipeline *pipeline);
	void destroy(VkPipeline *pipeline);
	void setInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo info);
	void setInputAssemblyState(VkPrimitiveTopology topology, VkBool32 restart_enable, VkPipelineInputAssemblyStateCreateFlags flags=0x0, void *pNext=nullptr);
	void setRasterizationState(VkPipelineRasterizationStateCreateInfo info);
	void setRasterizationState(VkPolygonMode mode, VkCullModeFlags cull, VkFrontFace face, 
							   VkPipelineRasterizationStateCreateFlags flags, 
							   void *pNext=nullptr, 
							   VkBool32 raster_discard_enable = VK_FALSE,
							   VkBool32 depth_clamp_enable = VK_FALSE,
							   VkBool32 depth_bias_enable = VK_FALSE,
							   float depth_bias_constant_factor=0.0f, 
							   float depth_bias_clamp=0.0f, 
							   float depth_bias_slope_factor=0.0f, 
							   float line_width=1.0f);
	void setColorBlendState(VkPipelineColorBlendStateCreateInfo info);
	void setDepthStencilState(VkPipelineDepthStencilStateCreateInfo info);
	void setViewportState(VkPipelineViewportStateCreateInfo info);
	void setViewportState(uint32_t nr_viewports, VkViewport *p_viewports, uint32_t nr_scissors, VkRect2D *p_scissors, VkPipelineViewportStateCreateFlags flags, void *pNext=nullptr);
	void setMultiSampleState(VkPipelineMultisampleStateCreateInfo info);
	void setDynamicState(VkPipelineDynamicStateCreateInfo info);
	void setDynamicState(uint32_t nr_dstates, VkDynamicState *p_dynamics, VkPipelineDynamicStateCreateFlags flags, void *pNext=nullptr);
	void setVertexInputState(VkPipelineVertexInputStateCreateInfo info);
};
}
#endif