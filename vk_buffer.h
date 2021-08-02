#ifndef __VK_BUFFER_H__
#define __VK_BUFFER_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <cstring>
#include "vk_context.h"

namespace VKEngine
{
	class Buffer
	{	
		private :
		Context* context = nullptr;
		VkDevice device = VK_NULL_HANDLE;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memory_properties;
		VkMemoryRequirements memory_requirements;
		VkDescriptorBufferInfo descriptor;
		
		public :
		void *data;

		private :
		void create(Context *_context, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memory_properties, VkDeviceSize _size, void *ptr);
		void allocate(VkDeviceSize offset = 0, VkDeviceSize size = 0);
		void bind(VkDeviceSize offset=0);
		void setupDescriptor(VkDeviceSize offset=0, VkDeviceSize size =VK_WHOLE_SIZE);
		
		public:
		Buffer();
		Buffer(Context *_context, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memory_properties, VkDeviceSize _size, void *ptr);
		~Buffer();
		void map(VkDeviceSize offset, VkDeviceSize _size);
		void unmap();
		void flush(VkDeviceSize offset, VkDeviceSize _size);
		void invalidate(VkDeviceSize offset, VkDeviceSize _size);
		void copyTo(void* dst, VkDeviceSize _size = VK_WHOLE_SIZE);
		void copyFrom(void* src, VkDeviceSize _size = VK_WHOLE_SIZE);
		void barrier(VkCommandBuffer command_buffer, 
					VkAccessFlags src_access, 
					VkAccessFlags dst_access, 
					VkPipelineStageFlags src_stage, 
					VkPipelineStageFlags dst_stage);
		void destroy();
		
		operator VkBuffer() const{
			return buffer;
		}
		operator VkBufferUsageFlags() const{
			return this->usage;
		}
	};
};

#endif