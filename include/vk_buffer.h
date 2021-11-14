#ifndef __VK_BUFFER_H__
#define __VK_BUFFER_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "vk_context.h"
#include "vk_infos.h"

using namespace std;

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
		VkMemoryRequirements memory_requirements;
		VkMemoryPropertyFlags memory_properties;

		public :
		void *data = nullptr;
		VkDescriptorBufferInfo descriptor;

		private :
		void allocate(VkDeviceSize offset = 0, VkDeviceSize size = 0);
		void bind(VkDeviceSize offset=0);
		public:
		Buffer();
		Buffer(Context *_context, 
				VkBufferUsageFlags _usage, 
				VkMemoryPropertyFlags _memory_properties,
				VkDeviceSize _size,
				void *ptr);
		~Buffer();
		void create(Context *_context, 
					VkBufferUsageFlags _usage,
					VkMemoryPropertyFlags _memory_properties,
					VkDeviceSize _size,
					void *ptr=nullptr
					);
		void map(VkDeviceSize offset = 0 , VkDeviceSize size = VK_WHOLE_SIZE);
		void unmap();
		void flush(VkDeviceSize offset, VkDeviceSize size);
		void invalidate(VkDeviceSize offset, VkDeviceSize size);
		void setupDescriptor(VkDeviceSize offset, VkDeviceSize size);
		void copyTo(void* dst, VkDeviceSize size = VK_WHOLE_SIZE);
		void copyFrom(void* src, VkDeviceSize size = VK_WHOLE_SIZE);
		VkBufferMemoryBarrier barrier(VkAccessFlags src_access, VkAccessFlags dst_access, uint32_t src_queue_index = VK_QUEUE_FAMILY_IGNORED, uint32_t dst_queue_index = VK_QUEUE_FAMILY_IGNORED);
		void destroy();
		
		VkBuffer getBuffer() const {
			return this->buffer;
		}

		VkBufferUsageFlags getBufferUsageFlags() const {
			return this->usage;
		}

		VkDeviceSize getSize() const {
			return this->size;
		}
	};
};

#endif