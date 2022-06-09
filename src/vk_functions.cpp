#ifndef __VK_FUNCTIONS_CPP__
#define __VK_FUNCTIONS_CPP__
#include "vk_functions.h"

using namespace std;

namespace VKEngine{

Buffer createStagingBuffer(Context *ctx, size_t mem_size)
{
	Buffer buffer(ctx, STAGING_BUFFER_USAGE, STAGING_BUFFER_PROPERTY, mem_size, nullptr);
	return buffer;
}

void enqueueCopyBuffer(CommandQueue *queue, void* src, Buffer *dst, size_t src_offset, size_t dst_offset, size_t size, bool is_blocking)
{
	VkFence fence=VK_NULL_HANDLE;
	VkCommandBuffer command=VK_NULL_HANDLE;
	if(is_blocking){
		queue->getContext()->createFence(&fence);
	}

	Buffer staging(queue->getContext(), STAGING_BUFFER_USAGE, STAGING_BUFFER_PROPERTY, size, src);
	queue->createCommandBuffer(&command, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	queue->beginCommandBuffer(command, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	queue->copyBuffer(command, &staging, dst, src_offset, dst_offset, size);
	queue->endCommandBuffer(command);
	if(fence != VK_NULL_HANDLE){
		queue->resetFences(&fence, 1);
		queue->submit(&command, 1, nullptr,nullptr, 0, nullptr, 0, fence);
		queue->waitFences(&fence,1);
		queue->getContext()->destroyFence(&fence);
	}else{
		queue->submit(&command, 1, nullptr,nullptr, 0, nullptr, 0, nullptr);
	}
	queue->free(command);
	staging.destroy();
}


void enqueueCopyBuffer(CommandQueue *queue, Buffer *src, void *dst, size_t src_offset, size_t dst_offset, size_t size, bool is_blocking)
{
	VkFence fence=VK_NULL_HANDLE;
	VkCommandBuffer command=VK_NULL_HANDLE;
	if(is_blocking){
		queue->getContext()->createFence(&fence);
	}
	Buffer staging(queue->getContext(), STAGING_BUFFER_USAGE, STAGING_BUFFER_PROPERTY, size, nullptr);
	queue->createCommandBuffer(&command, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	queue->beginCommandBuffer(command, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	queue->copyBuffer(command, src, &staging, src_offset, dst_offset, size);
	queue->endCommandBuffer(command);
	if(fence != VK_NULL_HANDLE){
		queue->resetFences(&fence, 1);
		queue->submit(&command, 1, nullptr,nullptr, 0, nullptr, 0, fence);
		queue->waitFences(&fence,1);
		queue->getContext()->destroyFence(&fence);
	}else{
		queue->submit(&command, 1, nullptr,nullptr, 0, nullptr, 0, nullptr);
	}
	queue->free(command);
	staging.copyTo(dst, size);
	staging.destroy();
}




}
#endif 