#ifndef __VK_QUEUE_CPP__
#define __VK_QUEUE_CPP__

#include "vk_queue.h"
namespace VKEngine{
	CommandQueue::CommandQueue(
		Context *_context,
		VkQueueFlagBits _type,
		VkCommandPool _pool)
	{
		context = _context;
		type = _type;
		pool = _pool;
	}

	CommandQueue::~CommandQueue(){
		destroy();
	}

	void CommandQueue::destroy(){
		if(!pool)
			vkDestroyCommandPool(device, pool, nullptr);
	}

	void CommandQueue::free( VkCommandBuffer command_buffer ){
		if(!command_buffer){
			vkFreeCommandBuffers(device, pool, 1, &command_buffer);
		}
	}
}
#endif