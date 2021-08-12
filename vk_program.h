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
	struct InputAssemblyState{
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPipelineInputAssemblyStateCreateFlags flags = 0;
		VkBool32 primitive_restart = VK_FALSE;
	};
	
	struct RasterizationState{
		VkPolygonMode polygon = VK_POLYGON_MODE_FILL;
		VkCullModeFlags cull =  VK_CULL_MODE_BACK_BIT;
		VkFrontFace face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		VkPipelineRasterizationStateCreateFlags flags =0;
	};

	struct ColorBlendAttachmentState{
		VkColorComponentFlags color_write_mask;
		VkBool32 blend_enable = VK_FALSE;
	};

	struct DepthStencilState{
		VkBool32 depth_test_enabled = VK_TRUE;
		VkBool32 depth_write_enabled = VK_TRUE;
		VkCompareOp depth_compare_OP = VK_COMPARE_OP_LESS_OR_EQUAL;
	};

	struct ViewportState{
		uint32_t nr_viewports = 1;
		uint32_t nr_scissors = 1;
		VkPipelineViewportStateCreateFlags flags = 0;
	};

	struct MultisampleState{
		VkSampleCountFlagBits rasterization_samples = VK_SAMPLE_COUNT_1_BIT;
		VkPipelineMultisampleStateCreateFlags flags = 0;
	};

	struct VertexInputState{
		vector<VkVertexInputAttributeDescription> attributes;
		vector<VkVertexInputBindingDescription> bindings;
	};

	struct DescriptorsInfo{
		VkDescriptorPool pool = VK_NULL_HANDLE;
		vector<VkDescriptorSet> sets;
		vector<VkDescriptorSetLayout> layouts;
		vector<VkDescriptorSetLayoutBinding> bindings;
	};

	class Program{
		public :
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		vector<VkDynamicState> dynamic_state_enabled = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		InputAssemblyState input_assembly_state;
		RasterizationState rasterization_state;
		ColorBlendAttachmentState color_blend_attachment_state;
		DepthStencilState depth_stencil_state;
		ViewportState viewport_state;
		MultisampleState multisample_state;
		VertexInputState vertex_input_state;
		vector<Shader> shaders;
		DescriptorsInfo descriptors;

		protected :
		Context *context = nullptr;
		VkDevice device = VK_NULL_HANDLE;
		VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
		
		public :
		explicit Program(Context *_context);
		~Program();
		void attachShader(const string file_path, VkShaderStageFlagBits stage);
		virtual void build(VkRenderPass render_pass, VkPipelineCache cache = VK_NULL_HANDLE);
		VkResult createDescriptorPool(vector<VkDescriptorPoolSize> pool_size, uint32_t max_set = 1);
		VkResult allocDescriptorSet(VkDescriptorSet *descriptor_set, uint32_t set_idx, uint32_t nr_alloc=1);
		void releaseDescriptorSet(VkDescriptorSet *descriptor_set);
		void setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings);
		void uniformUpdate(VkDescriptorSet descriptor_set, 
						   VkDescriptorType type, 
						   uint32_t binding_idx, 
						   VkDescriptorBufferInfo *buffer_info,
						   VkDescriptorImageInfo *image_info);
		void destroy();
	};
}

#endif