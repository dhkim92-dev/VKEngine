#ifndef __VK_QUEUE_H__
#define __VK_QUEUE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "vk_context.h"
#include "vk_buffer.h"
#include "vk_image.h"
#include "vk_queue.h"
#include "vk_image.h"
#include "vk_buffer.h"
#include "vk_infos.h"

using namespace std;

namespace VKEngine{
	class CommandQueue{
		public:
		private:
		Context *context = nullptr;
		VkDevice device = VK_NULL_HANDLE;
		VkQueueFlagBits type;
		VkCommandPool pool = VK_NULL_HANDLE;
		VkQueue queue = VK_NULL_HANDLE;
		VkFence fence = VK_NULL_HANDLE;
		
		private :
		void createCommandQueue();

		public:
		//explicit CommandQueue();
		explicit CommandQueue(Context *_context, VkQueueFlagBits _type, VkCommandPool _pool);
		explicit CommandQueue(Context *_context, VkQueueFlagBits _type);
		~CommandQueue();
		void destroy();
		void enqueueCopy(void *src, Buffer *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size);
		void enqueueCopy(Buffer *src, void *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size);
		void enqueueCopy(Buffer *src, Buffer *dst,VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size);
		//void enqueueCopy(Image *src, Buffer *dst, VkDeviceSize from, VkDeviceSize to);
		//void enqueueCopy(Buffer *src, Image *dst, VkDeviceSize from, VkDeviceSize to);
		//void enqueueCopy(Image *src, void *dst, VkDeviceSize from, VkDeviceSize to);
		VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandBufferUsageFlags usage=0x0);
		void beginCommandBuffer(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage=0x0);
		void endCommandBuffer(VkCommandBuffer command_buffer);
		void submit(VkCommandBuffer command_buffer, VkBool32 fenced = false);
		//void submit(VkCommandBuffer command_buffer, VkSubmitInfo submit_info, VkBool32 fenced = false);
		void free(VkCommandBuffer command_buffer);
		void resetFence();
		void waitFence();
	};
}

#endif