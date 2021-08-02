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
	}

	void Buffer::bind(VkDeviceSize ){
		vkBindBufferMemory(device, buffer, memory, 0);
	}

	void Buffer::allocate(VkDeviceSize _offest = 0, VkDeviceSize _size = 0){
		VkMemoryAllocateInfo malloc_CI = infos::memoryAllocateInfo();		
		vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
		malloc_CI.allocationSize = size;
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


}

#endif