#ifndef __VK_QUEUE_CPP__
#define __VK_QUEUE_CPP__

#include "vk_queue.h"

namespace VKEngine{
	CommandQueue::CommandQueue(){};
	CommandQueue::CommandQueue(VkDevice _device,
							   VkQueue _queue, 
							   VkQueueFlagBits _type,
							   VkCommandPool _pool)
	: device(_device), type(_type), queue(_queue), pool(_pool){
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