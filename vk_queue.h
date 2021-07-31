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
		
		public:
		//explicit CommandQueue();
		explicit CommandQueue(Context *_context, VkQueueFlagBits _type, VkCommandPool _pool);
		~CommandQueue();
		void destroy();
		void enqueueCopy(void *src, Buffer *dst, VkDeviceSize from, VkDeviceSize to);
		void enqueueCopy(Buffer *src, Buffer *dst,VkDeviceSize from, VkDeviceSize to);
		void enqueueCopy(Image *src, Buffer *dst, VkDeviceSize from, VkDeviceSize to);
		void enqueueCopy(Buffer *src, Image *dst, VkDeviceSize from, VkDeviceSize to);
		void enqueueCopy(Buffer *src, void *dst, VkDeviceSize from, VkDeviceSize to);
		void enqueueCopy(Image *src, void *dst, VkDeviceSize from, VkDeviceSize to);
		VkCommandBuffer createOnetimeCommandBuffer();
		VkCommandBuffer createCommandBuffer();
		void beginCommandBuffer(VkCommandBuffer command_buffer);
		void endCommandBuffer(VkCommandBuffer command_buffer);
		void submit(VkCommandBuffer command_buffer);
		void free(VkCommandBuffer command_buffer);
	};
}

#endif