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
#include "vk_compute.h"

using namespace std;

namespace VKEngine{

	struct WorkGroupSize{
		uint32_t x=1;
		uint32_t y=1;
		uint32_t z=1;
	};

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

		CommandQueue();
		CommandQueue(Context *_context, VkQueueFlagBits _type);
		~CommandQueue();

		void create(Context *_context, VkQueueFlagBits _type);
		void destroy();


		VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandBufferUsageFlags usage=0x0, bool begin=false);
		void beginCommandBuffer(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage=0x0);
		void endCommandBuffer(VkCommandBuffer command_buffer);

		VkResult submit(VkCommandBuffer* commands, uint32_t nr_commands,
					VkPipelineStageFlags wait_signal_stage_mask,
					VkSemaphore *wait_smps, uint32_t nr_wait_smps, 
					VkSemaphore *signal_smps, uint32_t nr_signal_smps, VkFence fence=VK_NULL_HANDLE);
		void copyBuffer(VkCommandBuffer cmd, Buffer *src, Buffer *dst, uint32_t src_offset, uint32_t dst_offset, uint32_t size);
		void copyBufferToImage(VkCommandBuffer command_buffer,
								Buffer *src, Image *dst, VkBufferImageCopy *region);
		void copyImageToBuffer(VkCommandBuffer command_buffer, Image *src, Buffer *dst, VkBufferImageCopy *region);
		void copyImage(VkCommandBuffer command_buffer, 
						Image *src, Image *dst, VkImageCopy *region);
		void bindDescriptorSets(VkCommandBuffer command, VkPipelineBindPoint bind_point,  VkPipelineLayout pipeline_layout, 
								uint32_t first_set, VkDescriptorSet* sets, uint32_t nr_sets, 
								uint32_t nr_dynamic_offset = 0, uint32_t* dynamic_offsets= nullptr);
		void dispatch(VkCommandBuffer cmd, uint32_t gx, uint32_t gy, uint32_t gz);
		void free(VkCommandBuffer command_buffer);
		VkResult resetFences(VkFence *fences, uint32_t nr_fences);
		VkResult waitFences(VkFence* fences, uint32_t nr_fences, bool wait_all=true, uint64_t timeout = UINT64_MAX);
		VkResult waitIdle();

		// 0.1.3 add
		void free(VkCommandBuffer *command_buffers, uint32_t nr_command_buffers);
		void bindPipeline(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point, VkPipeline pipeline);
		void setEvent(VkCommandBuffer command_buffer, VkEvent event, VkPipelineStageFlags stage_mask);
		void waitEvents(VkCommandBuffer command_buffer, VkEvent *p_wait_events, uint32_t nr_wait_events, 
						VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask, 
						VkMemoryBarrier *memory_barriers ,uint32_t nr_memory_barrier,
						VkBufferMemoryBarrier *buffer_memory_barrier, uint32_t nr_buffer_memory_barrier,
						VkImageMemoryBarrier *image_memory_barrier, uint32_t nr_image_memory_barrier);
		void resetEvent(VkCommandBuffer command_buffer, VkEvent event, VkPipelineStageFlags stage_mask);

		void barrier(
			VkCommandBuffer command_buffer, 
			VkPipelineStageFlags src_masks, VkPipelineStageFlags dst_masks,
			VkDependencyFlags dependencies,
			VkMemoryBarrier *p_memory_barriers, uint32_t nr_memory_barriers,
			VkBufferMemoryBarrier * p_buffer_barriers, uint32_t nr_buffer_barriers,
			VkImageMemoryBarrier *p_image_barriers, uint32_t nr_image_barriers);

		//----------------------------------------------------------------------------- legacy functions
		void enqueueCopy(void *src, Buffer *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking=true );
		void enqueueCopy(Buffer *src, void *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking=true);
		void enqueueCopy(Buffer *src, Buffer *dst,VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking=true);
		//---------------------------------------------------------------------------------------------------------------
		
		//@deprecated
		VkFence createFence(VkFenceCreateFlagBits flag = VK_FENCE_CREATE_SIGNALED_BIT);
		void bindKernel(VkCommandBuffer command_buffer, Kernel *kernel);
		void destroyFence(VkFence fence);
		operator VkQueue() const {
			return queue;
		}
		operator VkCommandPool() const {
			return pool;
		}
	};
}

#endif