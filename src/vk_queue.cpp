#ifndef __VK_QUEUE_CPP__
#define __VK_QUEUE_CPP__

#include "vk_queue.h"
namespace VKEngine{
	CommandQueue::CommandQueue(Context * _context, VkQueueFlagBits _type){
		context = _context;
		device = _context->getDevice();
		type = _type;
		//pool = context->getCommandPool(type);
		pool = context->createCommandPool(_type);
		LOG("CommandQueue::CommandQueue context : %p \n", context);
		LOG("CommandQueue::CommandQueue device : %p \n", device);
		LOG("CommandQueue::CommandQueue pool : %p \n", pool);
		createCommandQueue();
	}

	CommandQueue::~CommandQueue(){
		LOG("CommandQueue::~CommandQueue()\n");
		destroy();
		LOG("CommandQueue::~CommandQueue() end\n");
	}

	void CommandQueue::destroy(){
		LOG("CommandQueue::destroy()\n");
		if(fence){
			vkDestroyFence(device, fence, nullptr);
			fence = VK_NULL_HANDLE;
		}
		if(pool){
			vkDestroyCommandPool(device, pool, nullptr);
			pool =VK_NULL_HANDLE;
		}

		LOG("CommandQueue::destroy() done\n");
	}

	void CommandQueue::createCommandQueue(){
		VkDeviceQueueCreateInfo queue_CI = infos::deviceQueueCreateInfo();
		QueueFamilyIndice indices = context->queue_family_indices;
		//uint32_t index = 0;

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

	void CommandQueue::free(VkCommandBuffer *command_buffers, uint32_t nr_command_buffers){
		vkFreeCommandBuffers(device, pool, nr_command_buffers, command_buffers);
		for(uint32_t i = 0 ; i < nr_command_buffers ; ++i){
			command_buffers[i] = VK_NULL_HANDLE;
		}
	}

	VkCommandBuffer CommandQueue::createCommandBuffer(VkCommandBufferLevel level, VkCommandBufferUsageFlags usage, bool begin){
		VkCommandBuffer command_buffer = VK_NULL_HANDLE;
		VkCommandBufferAllocateInfo command_buffer_AI = infos::commandBufferAllocateInfo(pool, level, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &command_buffer_AI, &command_buffer));
		if(begin){
			beginCommandBuffer(command_buffer, usage);
		}
		return command_buffer;
	}

	VkResult CommandQueue::beginCommandBuffer(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage){
		VkCommandBufferBeginInfo command_buffer_BI = infos::commandBufferBeginInfo();
		if(usage & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT){
			command_buffer_BI.flags=usage;
		}
		return vkBeginCommandBuffer(command_buffer, &command_buffer_BI);
	}

	VkResult CommandQueue::endCommandBuffer(VkCommandBuffer command_buffer){
		return vkEndCommandBuffer(command_buffer);
	}

	void CommandQueue::copyBuffer(VkCommandBuffer command_buffer, Buffer *src, Buffer *dst, uint32_t src_offset, uint32_t dst_offset, uint32_t size ){
		VkBufferCopy copy;
		copy.dstOffset = dst_offset;
		copy.srcOffset = src_offset;
		copy.size = size;
		vkCmdCopyBuffer(command_buffer, 
						src->getBuffer(), 
						dst->getBuffer(), 
						1, &copy );
	}

	void CommandQueue::copyImage(VkCommandBuffer command_buffer, 
								Image *src, Image *dst, VkImageCopy *region){
		vkCmdCopyImage(command_buffer, src->getImage(), src->getLayout(), dst->getImage(), dst->getLayout(), 1,region);
	}

	void CommandQueue::copyBufferToImage(VkCommandBuffer command_buffer, Buffer *src, Image *dst, VkBufferImageCopy *region){
		vkCmdCopyBufferToImage(command_buffer, src->getBuffer(), dst->getImage(), dst->getLayout(), 1, region);
	}

	void CommandQueue::copyImageToBuffer(VkCommandBuffer command_buffer, Image *src, Buffer *dst, VkBufferImageCopy *region){
		vkCmdCopyImageToBuffer(command_buffer, src->getImage(), src->getLayout(), dst->getBuffer(), 1, region);
	}

	void CommandQueue::bindKernel(VkCommandBuffer command_buffer, Kernel *kernel){
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, kernel->pipeline);
		// vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
								// kernel->layout, 0, 
								// 1, &kernel->descriptors.set, 
								// 0, nullptr );
	}

	void CommandQueue::bindPipeline(VkCommandBuffer command, VkPipelineBindPoint bind_point, VkPipeline pipeline){
		vkCmdBindPipeline(command, bind_point ,pipeline);;
	}

	void CommandQueue::bindDescriptorSets(VkCommandBuffer command, VkPipelineBindPoint bind_point, VkPipelineLayout pipeline_layout, 
							uint32_t first_set, VkDescriptorSet* sets, uint32_t nr_sets, uint32_t nr_dynamic_offset, uint32_t* dynamic_offsets){
		vkCmdBindDescriptorSets(command, bind_point, pipeline_layout, first_set,  nr_sets,  sets, nr_dynamic_offset, dynamic_offsets);
	}

	void CommandQueue::dispatch(VkCommandBuffer command_buffer, uint32_t gx, uint32_t gy, uint32_t gz){
		vkCmdDispatch(command_buffer, gx, gy, gz);
	}

	VkResult CommandQueue::submit(VkCommandBuffer* commands, uint32_t nr_commands,
					VkPipelineStageFlags *wait_signal_stage_mask,
					VkSemaphore *wait_smps, uint32_t nr_wait_smps,
					VkSemaphore *signal_smps, uint32_t nr_signal_smps, VkFence fence){
		VkSubmitInfo info = infos::submitInfo();
		info.commandBufferCount = nr_commands;
		info.pCommandBuffers = commands;
		info.signalSemaphoreCount = nr_signal_smps;
		info.pSignalSemaphores = signal_smps;
		info.waitSemaphoreCount = nr_wait_smps;
		info.pWaitSemaphores = wait_smps;
		info.pWaitDstStageMask = wait_signal_stage_mask;
		return vkQueueSubmit(queue, 1, &info, fence);
	}

	VkFence CommandQueue::createFence(VkFenceCreateFlagBits flag){
		VkFenceCreateInfo info = infos::fenceCreateInfo(flag);
		VkDevice device = context->getDevice();
		VkFence fence;
		VK_CHECK_RESULT(vkCreateFence(device, &info, nullptr, &fence));
		return fence;
	}
	VkResult CommandQueue::resetFences(VkFence *fences, uint32_t nr_fences){
		VkDevice device = context->getDevice();
		return vkResetFences(device, nr_fences, fences);
	}	
	
	VkResult CommandQueue::waitIdle(){
		return vkQueueWaitIdle(queue);
	}

	VkResult CommandQueue::waitFences(VkFence* fences, uint32_t nr_fences, bool wait_all, uint64_t timeout){
		return vkWaitForFences(device, nr_fences, fences, wait_all, timeout);
	}

	void CommandQueue::destroyFence(VkFence fence){
		VkDevice device = context->getDevice();
		vkDestroyFence(device, fence, nullptr);
	}

	// ------------------------- Legacy Functions ------------------------------------
	void CommandQueue::enqueueCopy(Buffer *src, Buffer *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking){
		/**
		 * copy src buffer to dst buffer,
		 */
		VkFence f;
		context->createFence(&f);//createFence();
		VkCommandBuffer command_buffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		beginCommandBuffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkBufferCopy region = {};
		region.size = size;
		region.srcOffset = src_offset;
		region.dstOffset = dst_offset;
		vkCmdCopyBuffer(command_buffer, src->getBuffer(), dst->getBuffer(),1, &region );
		endCommandBuffer(command_buffer);
		VkSubmitInfo submit_info = infos::submitInfo();
		submit_info.pCommandBuffers = &command_buffer;
		submit_info.commandBufferCount = 1;
		resetFences(&f, 1);
		vkQueueSubmit(queue, 1, &submit_info, f);
		waitFences(&f,1);
		context->destroyFence(&f);
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
	// ------------------------- Legacy Functions ------------------------------------

	void CommandQueue::setEvent(VkCommandBuffer command_buffer, VkEvent event, VkPipelineStageFlags stage_mask){
		vkCmdSetEvent(command_buffer, event, stage_mask);
	}

	void CommandQueue::waitEvents(
	VkCommandBuffer command_buffer, 
	VkEvent *p_wait_events, uint32_t nr_wait_events, 
	VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask, 
	VkMemoryBarrier *memory_barriers ,uint32_t nr_memory_barrier,
	VkBufferMemoryBarrier *buffer_memory_barrier, uint32_t nr_buffer_memory_barrier,
	VkImageMemoryBarrier *image_memory_barrier, uint32_t nr_image_memory_barrier){
		vkCmdWaitEvents( command_buffer, nr_wait_events, p_wait_events, src_stage_mask, dst_stage_mask,
		nr_memory_barrier, memory_barriers,
		nr_buffer_memory_barrier, buffer_memory_barrier,
		nr_image_memory_barrier, image_memory_barrier);
	}

	void CommandQueue::resetEvent(VkCommandBuffer command_buffer, VkEvent event, VkPipelineStageFlags stage_mask){
		vkCmdResetEvent(command_buffer, event, stage_mask);
	}

	void CommandQueue::barrier(
		VkCommandBuffer command_buffer, 
		VkPipelineStageFlags src_masks, VkPipelineStageFlags dst_masks,
		VkDependencyFlags dependencies,
		VkMemoryBarrier *p_memory_barriers, uint32_t nr_memory_barriers,
		VkBufferMemoryBarrier * p_buffer_barriers, uint32_t nr_buffer_barriers,
		VkImageMemoryBarrier *p_image_barriers, uint32_t nr_image_barriers){
		vkCmdPipelineBarrier(command_buffer, src_masks, dst_masks, dependencies, nr_memory_barriers, p_memory_barriers, nr_buffer_barriers, p_buffer_barriers, nr_image_barriers, p_image_barriers);
	}
	

}
#endif