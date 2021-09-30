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

		void bindKernel(VkCommandBuffer command_buffer, Kernel *kernel);
		VkResult submit(VkCommandBuffer* commands, uint32_t nr_commands,
					VkPipelineStageFlags wait_signal_stage_mask,
					VkSemaphore *wait_smps, uint32_t nr_wait_smps, 
					VkSemaphore *signal_smps, uint32_t nr_signal_smps, VkFence fence=VK_NULL_HANDLE);
		void copyBuffer(VkCommandBuffer cmd, Buffer *src, Buffer *dst, uint32_t src_offset, uint32_t dst_offset, uint32_t size);
		void bindDescriptorSets(VkCommandBuffer command, VkPipelineBindPoint bind_point,  VkPipelineLayout pipeline_layout, 
								uint32_t first_set, VkDescriptorSet* sets, uint32_t nr_sets, 
								uint32_t nr_dynamic_offset = 0, uint32_t* dynamic_offsets= nullptr);
		void dispatch(VkCommandBuffer cmd, uint32_t gx, uint32_t gy, uint32_t gz);
		//void copyImage();
		//void copyImage();
		//void copyImage();
		void free(VkCommandBuffer command_buffer);
		VkFence createFence(VkFenceCreateFlagBits flag = VK_FENCE_CREATE_SIGNALED_BIT);
		VkResult resetFences(VkFence *fences, uint32_t nr_fences);
		VkResult waitFences(VkFence* fences, uint32_t nr_fences, bool wait_all=true, uint64_t timeout = UINT64_MAX);
		void destroyFence(VkFence fence);
		VkResult waitIdle();

		//----------------------------------------------------------------------------- legacy functions, will be deleted
		void ndRangeKernel(Kernel *kernel, WorkGroupSize gw);
		void enqueueCopy(void *src, Buffer *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking=true );
		void enqueueCopy(Buffer *src, void *dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking=true);
		void enqueueCopy(Buffer *src, Buffer *dst,VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size, bool is_blocking=true);
		//void enqueueCopy(Image *src, Buffer *dst, VkDeviceSize from, VkDeviceSize to);
		//void enqueueCopy(Buffer *src, Image *dst, VkDeviceSize from, VkDeviceSize to);
		//void enqueueCopy(Image *src, void *dst, VkDeviceSize from, VkDeviceSize to);
		//---------------------------------------------------------------------------------------------------------------
		operator VkQueue() const {
			return queue;
		}
		operator VkCommandPool() const {
			return pool;
		}
	};
}

#endif