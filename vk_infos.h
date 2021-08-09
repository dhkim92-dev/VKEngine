#ifndef __VK_INFOS_H__
#define __VK_INFOS_H__
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <iostream>
#include "vk_utils.h"
using namespace std;


namespace VKEngine
{
	namespace infos
	{
		inline VkApplicationInfo appCreateInfo(){
			VkApplicationInfo app_info = {};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    		app_info.pApplicationName = nullptr;
		    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		    app_info.pEngineName = nullptr;
		    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		    app_info.apiVersion = VK_API_VERSION_1_2;
		    return app_info;
		}

		inline VkInstanceCreateInfo instanceCreateInfo(){
			VkInstanceCreateInfo instance_create_info = {};
			instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instance_create_info.pApplicationInfo = nullptr;
			return instance_create_info;
		}

		inline VkDeviceQueueCreateInfo deviceQueueCreateInfo(){
			VkDeviceQueueCreateInfo device_queue_create_info = {};
			device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_info.queueCount = 1;
			float queue_priority = 1.0f;
			device_queue_create_info.pQueuePriorities = &queue_priority;
			return device_queue_create_info;	
		}

		inline VkDeviceCreateInfo deviceCreateInfo(){
			VkDeviceCreateInfo logical_device_create_info = {};
			logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			logical_device_create_info.queueCreateInfoCount = 1;
			logical_device_create_info.enabledExtensionCount = 0;
			return logical_device_create_info;
		}

		inline VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queue_family_index, VkCommandPoolCreateFlags create_info_flags){
			VkCommandPoolCreateInfo command_pool_create_info = {};
			command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			command_pool_create_info.queueFamilyIndex = queue_family_index;
			command_pool_create_info.flags = create_info_flags;
			return command_pool_create_info;
		}

		inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool& command_pool, VkCommandBufferLevel level, uint32_t nr_buffer){
			VkCommandBufferAllocateInfo cmd_buf_alloc_info={};
			
			cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd_buf_alloc_info.commandPool = command_pool;
			cmd_buf_alloc_info.level = level;
			cmd_buf_alloc_info.commandBufferCount = nr_buffer;
			return cmd_buf_alloc_info;
		}

		inline VkCommandBufferBeginInfo commandBufferBeginInfo()
		{
			VkCommandBufferBeginInfo cmd_buf_begin_info = {};
			cmd_buf_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			return cmd_buf_begin_info;
		}

		inline VkBufferCreateInfo bufferCreateInfo(VkBufferUsageFlags flags, VkDeviceSize size){
			VkBufferCreateInfo buffer_create_info = {};
			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.usage = flags;
			buffer_create_info.size = size;
			return buffer_create_info;
		}

		inline VkMemoryAllocateInfo memoryAllocateInfo(){
			VkMemoryAllocateInfo memory_allocate_info ={};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			return memory_allocate_info;
		}

		inline VkMappedMemoryRange mappedMemoryRange(){
			VkMappedMemoryRange mapped_memory_range ={};
			mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			return mapped_memory_range;
		}

		inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
				VkDescriptorType descriptor_type,
				VkShaderStageFlags stage_flag,
				uint32_t bind_index,
				uint32_t nr_descriptor=1
			)
		{
			VkDescriptorSetLayoutBinding binding ={};
			binding.descriptorType = descriptor_type;
			binding.stageFlags = stage_flag;
			binding.binding = bind_index;
			binding.descriptorCount = nr_descriptor;
			return binding;
		}

		inline VkDescriptorPoolSize descriptorPoolSize(
			VkDescriptorType type,
			uint32_t nr_descriptor)
		{
			VkDescriptorPoolSize descriptor_pool_size={};
			descriptor_pool_size.type = type;
			descriptor_pool_size.descriptorCount = nr_descriptor;
			return descriptor_pool_size;
		}

		inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
			uint32_t poolSizeCount,
			VkDescriptorPoolSize* pPoolSizes,
			uint32_t maxSets)
		{
			VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.poolSizeCount = poolSizeCount;
			descriptorPoolInfo.pPoolSizes = pPoolSizes;
			descriptorPoolInfo.maxSets = maxSets;
			return descriptorPoolInfo;
		}

		inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const VkDescriptorSetLayoutBinding *p_bindings,
			uint32_t nr_bind
		 ){
			VkDescriptorSetLayoutCreateInfo info={};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.pBindings = p_bindings;
			info.bindingCount = nr_bind;
			return info;
		}

		inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(vector<VkDescriptorSetLayoutBinding>& bindings)
		{
			return descriptorSetLayoutCreateInfo(bindings.data(), static_cast<uint32_t>(bindings.size()));
		}

		inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(VkDescriptorPool pool, const VkDescriptorSetLayout *p_set_layouts, uint32_t nr_descriptor)
		{
			VkDescriptorSetAllocateInfo alloc_info ={};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = pool;
			alloc_info.pSetLayouts = p_set_layouts;
			alloc_info.descriptorSetCount = nr_descriptor;
			return alloc_info;
		}

		inline VkWriteDescriptorSet writeDescriptorSet(
			VkDescriptorSet dest,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorBufferInfo *buffer_info,
			VkDescriptorImageInfo *image_info,
			uint32_t nr_descriptor = 1
		){
			VkWriteDescriptorSet write_descriptor_set = {};
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.dstSet = dest;
			write_descriptor_set.descriptorType = type;
			write_descriptor_set.dstBinding = binding;
			write_descriptor_set.pBufferInfo = buffer_info;
			write_descriptor_set.pImageInfo = image_info;
			write_descriptor_set.descriptorCount = nr_descriptor;
			return write_descriptor_set;
		}

		inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			const VkDescriptorSetLayout *p_set_layout,
			uint32_t nr_layouts
		){
			VkPipelineLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.pSetLayouts = p_set_layout;
			info.setLayoutCount = nr_layouts;
			return info;
		}

		inline VkComputePipelineCreateInfo computePipelineCreateInfo(
			VkPipelineLayout layout,
			VkPipelineCreateFlags flags = 0
		)
		{
			VkComputePipelineCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			info.layout = layout;
			info.flags = flags;
			return info;
		}

		inline VkPipelineShaderStageCreateInfo shaderStageCreateInfo(string entry_point_name, VkShaderModule &module, VkShaderStageFlagBits stage=VK_SHADER_STAGE_COMPUTE_BIT)
		{
			VkPipelineShaderStageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.module = module;
			info.pName = entry_point_name.c_str();
			info.stage = stage;
			return info;
		}	
		
		inline VkPipelineShaderStageCreateInfo shaderStageCreateInfo(const char* entry_point_name, VkShaderModule& module, VkShaderStageFlagBits stage=VK_SHADER_STAGE_COMPUTE_BIT)
		{
			VkPipelineShaderStageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.module = module;
			info.pName = entry_point_name;
			info.stage = stage;
			return info;
		}

		inline VkImageViewCreateInfo imageViewCreateInfo()
		{
			VkImageViewCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			return info;
		}


		inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags bits){
			VkFenceCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.flags = bits;
			return info;
		}

		inline VkSubmitInfo submitInfo(){
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			return info;
		}

		inline VkBufferMemoryBarrier bufferMemoryBarrier()
		{
			VkBufferMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			return barrier;
		}
		
		inline VkImageCreateInfo imageCreateInfo(){
			VkImageCreateInfo image_create_info = {};
			image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			return image_create_info;
		}
	
		inline VkImageCreateInfo imageCreateInfo(VkImageType img_type, VkFormat img_format, VkImageCreateFlags flags, VkImageUsageFlags usage){
			VkImageCreateInfo image_create_info = {};
			image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_create_info.imageType = img_type;
			image_create_info.format = img_format;
			image_create_info.mipLevels = 1;
			image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			image_create_info.flags = flags;
			image_create_info.usage = usage;
			image_create_info.arrayLayers = 1;
			image_create_info.tiling=VK_IMAGE_TILING_OPTIMAL;
			image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
			return image_create_info;
		}

		inline VkSamplerCreateInfo samplerCreateInfo(){
			VkSamplerCreateInfo sampler_info = {};
			sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler_info.maxAnisotropy = 1.0f;
			return sampler_info;
		}

		inline VkImageMemoryBarrier imageMemoryBarrier()
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			return barrier;
		}

		inline VkImageMemoryBarrier imageMemoryBarrier(VkImageLayout old_layout,  VkImageLayout new_layout)
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = old_layout;
			barrier.newLayout = new_layout;

			switch(old_layout){
				case VK_IMAGE_LAYOUT_UNDEFINED :
					barrier.srcAccessMask= 0 ;
					break;
				case VK_IMAGE_LAYOUT_PREINITIALIZED :
					barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL :
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL :
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
					barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
					barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL :
					barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				default :
					break;
			}

			switch(new_layout){
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL :
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : 
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
					barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: 
					barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL :
					if(barrier.srcAccessMask == 0 ){
						barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
					}
					barrier.dstAccessMask =  VK_ACCESS_SHADER_READ_BIT;
					break;
				default : 
					break;
			}

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			return barrier;
		}

		inline VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo()
		{
			VkDebugUtilsMessengerCreateInfoEXT create_info;
			create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			return create_info;
		}

		inline VkSwapchainCreateInfoKHR swapchainCreateInfo(){
			VkSwapchainCreateInfoKHR info = {};
			info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			return info;
		}

		inline VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags , VkBool32 primitive_restart){
			VkPipelineInputAssemblyStateCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			info.topology = topology;
			info.flags = flags;
			info.primitiveRestartEnable = primitive_restart;
			return info;
		}

		inline VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(
			VkPolygonMode poly_mode,
			VkCullModeFlags cull_mode,
			VkFrontFace face,
			VkPipelineRasterizationStateCreateFlags flags = 0)
		{
			VkPipelineRasterizationStateCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			info.polygonMode = poly_mode;
			info.cullMode = cull_mode;
			info.frontFace = face;
			info.flags = flags;
			info.depthClampEnable = VK_FALSE;
			info.lineWidth = 1.0f;
			return info;
		}

		inline VkPipelineColorBlendAttachmentState colorBlendAttachmentState(VkColorComponentFlags color_write_mask, VkBool32 blend){
			VkPipelineColorBlendAttachmentState state = {};
			state.blendEnable = blend;
			state.colorWriteMask = color_write_mask;
			return state;
		}

		inline VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo(
			uint32_t nr_attachment,
			const VkPipelineColorBlendAttachmentState *pAttachments
		){
			VkPipelineColorBlendStateCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			info.attachmentCount = nr_attachment;
			info.pAttachments = pAttachments;
			return info;
		}

		inline VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo( 
			VkBool32 depth_test, 
			VkBool32 depth_write, VkCompareOp depth_cmp_op){
			VkPipelineDepthStencilStateCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			info.depthTestEnable= depth_test;
			info.depthWriteEnable = depth_write;
			info.depthCompareOp = depth_cmp_op;
			info.back.compareOp = VK_COMPARE_OP_ALWAYS;
			return info;
		}

		inline VkPipelineViewportStateCreateInfo viewportStateCreateInfo(
			uint32_t nr_viewports,
			uint32_t nr_scissors,
			VkPipelineViewportStateCreateFlags flags = 0
		){
			VkPipelineViewportStateCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			info.flags = flags;
			info.viewportCount = nr_viewports;
			info.scissorCount = nr_scissors;
			return info;
		}

		inline VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo(
			VkSampleCountFlagBits rasterization_sample,
			VkPipelineMultisampleStateCreateFlags flags = 0
		){
			VkPipelineMultisampleStateCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			info.rasterizationSamples = rasterization_sample;
			info.flags = flags;
			return info;
		}

		inline VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo(
			const vector<VkDynamicState> & states,
			VkPipelineDynamicStateCreateFlags flags = 0
		){
			VkPipelineDynamicStateCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			info.pDynamicStates = states.data();
			info.dynamicStateCount = states.size();
			info.flags = flags;
			return info;
		}

		inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
			VkPipelineLayout layout,
			VkRenderPass render_pass,
			VkPipelineCreateFlags flags = 0
		){
			VkGraphicsPipelineCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			info.layout = layout;
			info.renderPass = render_pass;
			info.flags = flags;
			info.basePipelineHandle = VK_NULL_HANDLE;
			info.basePipelineIndex = -1;
			return info;
		}

		inline VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(vector<VkVertexInputAttributeDescription>& attributes, vector<VkVertexInputBindingDescription>& bindings){
			VkPipelineVertexInputStateCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			
			if(attributes.size() == 0){
				info.pVertexAttributeDescriptions = nullptr;
			}else{
				info.pVertexAttributeDescriptions = attributes.data() ;
			}
			
			if(bindings.size() == 0 ){
				info.pVertexBindingDescriptions = nullptr;
			}else{
				info.pVertexBindingDescriptions = bindings.data();
			}

			info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
			info.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
			return info;
		}
		
		inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(VkRenderPass render_pass){
			VkGraphicsPipelineCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			info.renderPass = render_pass;
			return info;
		}

		inline VkRenderPassCreateInfo renderPassCreateInfo(){
			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			return info;
		}
	};
};

#endif
