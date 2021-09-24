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
		device = VkDevice(*context);
		type = _type;
		pool = context->getCommandPool(type);
		LOG("CommandQueue::CommandQueue context : %p \n", context);
		LOG("CommandQueue::CommandQueue device : %p \n", device);
		LOG("CommandQueue::CommandQueue pool : %p \n", pool);
		createCommandQueue();
	}

	CommandQueue::~CommandQueue(){
		destroy();
	}

	void CommandQueue::destroy(){
		if(fence){
			vkDestroyFence(device, fence, nullptr);
		}
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
		if(command_buffer){
			vkFreeCommandBuffers(device, pool, 1, &command_buffer);
		}
	}

	VkCommandBuffer CommandQueue::createCommandBuffer(VkCommandBufferLevel level, VkCommandBufferUsageFlags usage){
		VkCommandBuffer command_buffer = VK_NULL_HANDLE;
		VkCommandBufferAllocateInfo command_buffer_AI = infos::commandBufferAllocateInfo(pool, level, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &command_buffer_AI, &command_buffer));
		return command_buffer;
	}

	void CommandQueue::beginCommandBuffer(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage){
		VkCommandBufferBeginInfo command_buffer_BI = infos::commandBufferBeginInfo();
		if(usage & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT){
			command_buffer_BI.flags=usage;
		}
		VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &command_buffer_BI));
	}

	void CommandQueue::endCommandBuffer(VkCommandBuffer command_buffer){
		VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer));
	}

	void CommandQueue::submit(VkCommandBuffer command_buffer, VkSemaphore *wait, VkSemaphore *signal, VkPipelineStageFlags *stage, VkBool32 fenced){
		VkSubmitInfo command_buffer_SI = infos::submitInfo();
		command_buffer_SI.commandBufferCount = 1;
		command_buffer_SI.pCommandBuffers = &command_buffer;

		if(wait != nullptr){
			command_buffer_SI.pWaitSemaphores = wait;
			command_buffer_SI.waitSemaphoreCount = 1;
			command_buffer_SI.pWaitDstStageMask = stage;
		}

		if(signal != nullptr){
			command_buffer_SI.pSignalSemaphores = signal;
			command_buffer_SI.signalSemaphoreCount = 1;
		}

		if(fenced == VK_TRUE){
			resetFence();
			VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &command_buffer_SI, fence));
			waitFence();
		}else{
			VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &command_buffer_SI, VK_NULL_HANDLE));
		}
		VK_CHECK_RESULT(vkQueueWaitIdle(queue));
	}

	void CommandQueue::submit(VkSubmitInfo submit_info, VkBool32 fenced){
		if(fenced == VK_TRUE){
			resetFence();
			VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submit_info, fence));
			waitFence();
		}else{
			VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE));
		}
		vkQueueWaitIdle(queue);
	}

	void CommandQueue::resetFence(){
		if(fence == VK_NULL_HANDLE){
			VkFenceCreateInfo fence_CI = infos::fenceCreateInfo(0);
			VK_CHECK_RESULT(vkCreateFence(device, &fence_CI, nullptr, &fence));
		}else{
			VK_CHECK_RESULT(vkResetFences(device, 1, &fence));
		}
	}

	void CommandQueue::waitFence(){
		VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
		//vkDestroyFence(device, fence, nullptr);
		//fence = VK_NULL_HANDLE;
	}

	void CommandQueue::enqueueCopy(Buffer *src, Buffer *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking){
		/**
		 * copy src buffer to dst buffer,
		 */
		VkCommandBuffer command_buffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		beginCommandBuffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkBufferCopy region = {};
		region.size = size;
		region.srcOffset = src_offset;
		region.dstOffset = dst_offset;
		vkCmdCopyBuffer(command_buffer, VkBuffer(*src), VkBuffer(*dst),1, &region );
		endCommandBuffer(command_buffer);

		submit(command_buffer, nullptr, nullptr, nullptr, (is_blocking ? VK_TRUE: VK_FALSE));
		free(command_buffer);
	}

	void CommandQueue::enqueueCopy(void *src, Buffer *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking){
		Buffer staging(context, 
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					size, src);
		enqueueCopy(&staging, dst, src_offset, dst_offset, size, is_blocking);
		staging.destroy();
	}

	void CommandQueue::enqueueCopy(Buffer* src, void *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking){
		Buffer staging(context, 
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					size, nullptr);	
		enqueueCopy(src, &staging, src_offset, dst_offset, size, is_blocking);
		staging.copyTo(dst, size);
		staging.destroy();
	}

	void CommandQueue::ndRangeKernel(Kernel *kernel, WorkGroupSize gw, VkBool32 fenced)
	{
		VkEvent event;
		VkSubmitInfo submit_info = infos::submitInfo();
		VkPipeline pipeline = kernel->pipeline;
		VkPipelineLayout layout = kernel->layout;
		VkPipelineBindPoint point = VK_PIPELINE_BIND_POINT_COMPUTE;
		VkDescriptorSet descriptor_set = kernel->descriptors.set;
		VkCommandBuffer command_buffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		beginCommandBuffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkCmdBindPipeline(command_buffer, point, pipeline);
		vkCmdBindDescriptorSets(command_buffer, point, layout, 0, 1, &descriptor_set, 0, nullptr);
		//LOG("dispatch size : %d %d %d\n", gw.x, gw.y, gw.z);
		vkCmdDispatch(command_buffer, gw.x, gw.y, gw.z);
		endCommandBuffer(command_buffer);
		submit_info.commandBufferCount =1;
		submit_info.pCommandBuffers = &command_buffer;
		submit(submit_info, fenced);
		free(command_buffer);
	}

}
#endif