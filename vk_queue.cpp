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
		device = VkDevice(*context);
		type = _type;
		pool = _pool;
		createCommandQueue();
	}

	CommandQueue::CommandQueue(Context * _context, VkQueueFlagBits _type){
		context = _context;
		device = VkDevice(*_context);
		type = _type;
		pool = context->getCommandPool(type);
		createCommandQueue();
	}

	CommandQueue::~CommandQueue(){
		destroy();
	}

	void CommandQueue::destroy(){
	}

	void CommandQueue::createCommandQueue(){
		VkDeviceQueueCreateInfo queue_CI = infos::deviceQueueCreateInfo();
		QueueFamilyIndice indices = context->findQueueFamilies();
		uint32_t index = 0;

		switch(type){
			case VK_QUEUE_GRAPHICS_BIT : 
				index = indices.graphics.value();
				break;
			case VK_QUEUE_COMPUTE_BIT :
				index = indices.compute.value();
				break;
			default :
				index = 0;
		}
		vkGetDeviceQueue(device, index, 0, &queue);
	}

	void CommandQueue::free( VkCommandBuffer command_buffer ){
		if(!command_buffer){
			vkFreeCommandBuffers(device, pool, 1, &command_buffer);
		}
	}
}
#endif