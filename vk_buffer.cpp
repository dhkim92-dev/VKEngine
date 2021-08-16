#ifndef __VK_BUFFER_CPP__
#define __VK_BUFFER_CPP__

#include "vk_buffer.h"

namespace VKEngine{
	Buffer::Buffer(){};
	Buffer::Buffer(
		Context* _context,
		VkBufferUsageFlags _usage,
		VkMemoryPropertyFlags _memory_properties,
		VkDeviceSize _size,
		void *ptr
		){
		create(_context, _usage, _memory_properties, _size, ptr);
	};
	void Buffer::create(
		Context *_context,
		VkBufferUsageFlags _usage,
		VkMemoryPropertyFlags _memory_properties,
		VkDeviceSize _size,
		void *ptr
	){
		context = _context;
		device = VkDevice(*_context);
		size = _size;
		usage = _usage;
		memory_properties = _memory_properties;
		VkBufferCreateInfo buffer_CI = infos::bufferCreateInfo(_usage, _size);
		buffer_CI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT( vkCreateBuffer(device, &buffer_CI, nullptr, &buffer) );
		allocate(0, _size);
		bind(0);
		if(ptr != nullptr){
			copyFrom(ptr, size);
		}
	}

	Buffer::~Buffer(){
		destroy();
	}

	void Buffer::bind(VkDeviceSize offset){
		vkBindBufferMemory(device, buffer, memory, offset);
	}

	void Buffer::allocate(VkDeviceSize _offest, VkDeviceSize _size){
		VkMemoryAllocateInfo malloc_CI = infos::memoryAllocateInfo();		
		vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
		//malloc_CI.allocationSize = size;
		malloc_CI.memoryTypeIndex=context->getMemoryType(memory_requirements.memoryTypeBits, memory_properties);
		malloc_CI.allocationSize = memory_requirements.size;
		VkMemoryAllocateFlagsInfoKHR malloc_FI = {};
		if(usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT){
			malloc_FI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			malloc_FI.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			malloc_CI.pNext = &malloc_FI;
		}
		VK_CHECK_RESULT(vkAllocateMemory(device, &malloc_CI, nullptr, &memory));
	}

	void Buffer::setupDescriptor(VkDeviceSize offset, VkDeviceSize _size){
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range = _size;
	}

	void Buffer::map(VkDeviceSize offset, VkDeviceSize _size){
		// LOG("map called\n");
		VK_CHECK_RESULT(vkMapMemory(device, memory, offset, _size, 0, &data));
	}

	void Buffer::unmap(){
		// LOG("unmap called");
		vkUnmapMemory(device, memory);
		data = nullptr;
	}

	void Buffer::flush(VkDeviceSize offset, VkDeviceSize _size){
		// LOG("flush called\n");
		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.offset = offset;
		range.size = _size;
		range.memory = memory;
		VK_CHECK_RESULT(vkFlushMappedMemoryRanges(device, 1, &range));
	}

	void Buffer::invalidate(VkDeviceSize offset, VkDeviceSize _size){
		// LOG("invalidate called\n");
		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.offset = offset;
		range.size = _size;
		range.memory = memory;
		VK_CHECK_RESULT(vkInvalidateMappedMemoryRanges(device, 1, &range));
	}

	void Buffer::copyTo(void* dst, VkDeviceSize _size){
		LOG("Buffer::copyTo : %d\n", _size);
		LOG("Buffer::allocSize : %d\n", memory_requirements.size);
		map(0, _size);
		assert(data);
		invalidate(0, _size);
		memcpy(dst, data, _size);
		unmap();
	}

	void Buffer::copyFrom(void *src, VkDeviceSize _size){
		// LOG("copyFrom called\n");
		assert(src);
		map(0, _size);
		if( !(memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) )
			flush(0,_size);
		memcpy(data, src, _size);
		unmap();
	}

	void Buffer::destroy(){
		if(buffer){
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = VK_NULL_HANDLE;
		}

		if(memory){
			vkFreeMemory(device, memory, nullptr);
			memory = VK_NULL_HANDLE;
		}
	}

	void Buffer::barrier(
		VkCommandBuffer command_buffer, 
		VkAccessFlags src_access, 
		VkAccessFlags dst_access, 
		VkPipelineStageFlags src_stage, 
		VkPipelineStageFlags dst_stage
	){	
		VkBufferMemoryBarrier barrier = infos::bufferMemoryBarrier();
		barrier.buffer=buffer;
		barrier.srcAccessMask = src_access;
		barrier.dstAccessMask = dst_access;
		barrier.size = size;
		vkCmdPipelineBarrier(
			command_buffer,
			src_stage, dst_stage,
			0,
			0, nullptr,
			1, &barrier,
			0, nullptr
		);
	}

}

#endif