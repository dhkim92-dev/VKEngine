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
	struct DescriptorsInfo{
		VkDescriptorPool pool = VK_NULL_HANDLE;
		vector<VkDescriptorSet> sets;
		vector<VkDescriptorSetLayout> layouts;
		vector<VkDescriptorSetLayoutBinding> bindings;
	};

	struct ShaderArgs{
		uint32_t binding_index;
		VkDescriptorType descriptor_type;
		VkDescriptorBufferInfo *buffer_info;
		VkDescriptorImageInfo *image_info;
	};

	struct GraphicsPipelineCreateInfo{
		vector<VkDynamicState> dynamic_state_enabled = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineInputAssemblyStateCreateInfo input_assembly = infos::inputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE );
		VkPipelineViewportStateCreateInfo viewport = infos::viewportStateCreateInfo(1,1,0);
		VkPipelineRasterizationStateCreateInfo rasterization = infos::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
		VkPipelineMultisampleStateCreateInfo multisample=infos::multisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		vector<VkPipelineColorBlendAttachmentState> color_blend_states = {infos::colorBlendAttachmentState(0xf, VK_FALSE)};
		VkPipelineDepthStencilStateCreateInfo depth_stencil= infos::depthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineDynamicStateCreateInfo dynamic_state = infos::dynamicStateCreateInfo(dynamic_state_enabled);
		VkPipelineColorBlendStateCreateInfo color_blend = infos::colorBlendStateCreateInfo(static_cast<uint32_t>(color_blend_states.size()), color_blend_states.data());
		VkPipelineTessellationStateCreateInfo tesselation={};
		VkPipelineVertexInputStateCreateInfo vertex_input;
		uint32_t subpass = 0;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo(VkRenderPass renderpass, VkPipelineLayout layout){
			VkGraphicsPipelineCreateInfo info = infos::graphicsPipelineCreateInfo(renderpass);
			info.layout = layout;
			info.pInputAssemblyState = &input_assembly;
			info.pColorBlendState = &color_blend;
			info.pMultisampleState = &multisample;
			info.pViewportState = &viewport;
			info.pRasterizationState = &rasterization;
			info.pDynamicState = &dynamic_state;
			info.pDepthStencilState = &depth_stencil;
			info.pColorBlendState = &color_blend;
			info.pTessellationState = &tesselation;
			info.subpass = subpass;
			info.pVertexInputState = &vertex_input;

			return info;
		}
	};

	class Program{
		public :
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		vector<Shader> shaders;
		DescriptorsInfo descriptors;
		GraphicsPipelineCreateInfo graphics;

		protected :
		Context *context = nullptr;
		VkDevice device = VK_NULL_HANDLE;
		VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

		public :
		explicit Program(Context *_context);
		~Program();
		void attachShader(const string file_path, VkShaderStageFlagBits stage);
		void build(VkRenderPass render_pass, VkPipelineCache cache = VK_NULL_HANDLE);
		VkResult createDescriptorPool(vector<VkDescriptorPoolSize> pool_size, uint32_t max_set = 1);
		VkResult allocDescriptorSet(VkDescriptorSet *descriptor_set, uint32_t set_idx, uint32_t nr_alloc=1);
		void releaseDescriptorSet(VkDescriptorSet *descriptor_set);
		void setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings);
		void uniformUpdate(VkDescriptorSet descriptor_set, 
						   VkDescriptorType type, 
						   uint32_t binding_idx, 
						   VkDescriptorBufferInfo *buffer_info,
						   VkDescriptorImageInfo *image_info);
		void setShaderArgs(uint32_t set_id, vector<ShaderArgs> args);
		void destroy();
	};
}
#endif