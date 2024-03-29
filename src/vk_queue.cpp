#ifndef __VK_QUEUE_CPP__
#define __VK_QUEUE_CPP__

#include "vk_queue.h"
namespace VKEngine{
	CommandQueue::CommandQueue(Context * _context, VkQueueFlagBits _type){
		context = _context;
		device = _context->getDevice();
		type = _type;
		context->createCommandPool(&pool, _type);
		LOG("CommandQueue::CommandQueue context : %p \n", context);
		LOG("CommandQueue::CommandQueue device : %p \n", device);
		LOG("CommandQueue::CommandQueue pool : %p \n", pool);
		createCommandQueue();
	}

	CommandQueue::~CommandQueue(){
		destroy();
	}

	void CommandQueue::destroy(){
		context->destroyCommandPool(&pool);
	}

	void CommandQueue::createCommandQueue(){
		VkDeviceQueueCreateInfo queue_CI = infos::deviceQueueCreateInfo();
		QueueFamilyIndice indices = context-> getQueueFamily();
		switch(type){
			case VK_QUEUE_GRAPHICS_BIT : 
				index = indices.graphics.value();
				break;
			case VK_QUEUE_COMPUTE_BIT :
				index = indices.compute.value();
                break;
            case VK_QUEUE_TRANSFER_BIT :
                index = indices.transfer.value();
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

	VkResult CommandQueue::createCommandBuffer(VkCommandBuffer *command_buffers, size_t count, VkCommandBufferLevel level, VkCommandBufferUsageFlags usage){
		VkCommandBufferAllocateInfo command_buffer_AI = infos::commandBufferAllocateInfo(pool, level, count);
		return vkAllocateCommandBuffers(context->getDevice(), &command_buffer_AI, command_buffers);
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

	void CommandQueue::copyImage(VkCommandBuffer command_buffer, Image *src, Image *dst, VkImageCopy *region){
		vkCmdCopyImage(command_buffer, src->getImage(), src->getLayout(), dst->getImage(), dst->getLayout(), 1,region);
	}

	void CommandQueue::copyBufferToImage(VkCommandBuffer command_buffer, Buffer *src, Image *dst, VkBufferImageCopy *region){
		vkCmdCopyBufferToImage(command_buffer, src->getBuffer(), dst->getImage(), dst->getLayout(), 1, region);
	}

	void CommandQueue::copyImageToBuffer(VkCommandBuffer command_buffer, Image *src, Buffer *dst, VkBufferImageCopy *region){
		vkCmdCopyImageToBuffer(command_buffer, src->getImage(), src->getLayout(), dst->getBuffer(), 1, region);
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
					VkSemaphore *signal_smps, uint32_t nr_signal_smps, 
					VkFence fence){
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

	VkResult CommandQueue::present(VkSwapchainKHR *swapchain, uint32_t swapchain_count, uint32_t *image_index, VkSemaphore* wait_smp)
	{
		VkPresentInfoKHR info ={};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pSwapchains = swapchain;
		info.swapchainCount = swapchain_count;
		info.pWaitSemaphores = wait_smp;
		info.waitSemaphoreCount = (wait_smp == nullptr) ? 0 : 1;
		info.pImageIndices = image_index;
		info.pResults=nullptr;
		info.pNext=nullptr;
		
		return vkQueuePresentKHR(queue, &info);
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

	void CommandQueue::setEvent(VkCommandBuffer command_buffer, VkEvent event, VkPipelineStageFlags stage_mask){
		vkCmdSetEvent(command_buffer, event, stage_mask);
	}

	void CommandQueue::waitEvents(
		VkCommandBuffer command_buffer, 
		VkEvent *p_wait_events, uint32_t nr_wait_events, 
		VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask, 
		VkMemoryBarrier *memory_barriers ,uint32_t nr_memory_barrier,
		VkBufferMemoryBarrier *buffer_memory_barrier, uint32_t nr_buffer_memory_barrier,
		VkImageMemoryBarrier *image_memory_barrier, uint32_t nr_image_memory_barrier)
	{
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
