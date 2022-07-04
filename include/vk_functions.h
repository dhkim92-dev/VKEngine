#ifndef __VK_FUNCTIONS_H__
#define __VK_FUNCTIONS_H__
#include <vector>
#include "vk_image.h"
#include "vk_buffer.h"
#include "vk_queue.h"
#include "vk_infos.h"
#include "vk_utils.h"

using namespace std;

#define STAGING_BUFFER_SRC VK_BUFFER_USAGE_TRANSFER_SRC_BIT 
#define STAGING_BUFFER_DST VK_BUFFER_USAGE_TRANSFER_DST_BIT
#define STAGING_BUFFER_USAGE VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
#define STAGING_BUFFER_PROPERTY VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT


namespace VKEngine{

Buffer createStagingBuffer(Context *ctx, size_t size);
void enqueueCopyBuffer(CommandQueue *queue, void* src, Buffer *dst, size_t src_offset, size_t dst_offset, size_t size, bool is_blocking=true);
void enqueueCopyBuffer(CommandQueue *queue, Buffer *src, void *dst, size_t src_offset, size_t dst_offset, size_t size, bool is_blocking=true);
}
#endif 