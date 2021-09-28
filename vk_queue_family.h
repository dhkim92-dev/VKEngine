#ifndef __VK_QUEUE_FAMILY_H__
#define __VK_QUEUE_FAMILY_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

using namespace std;

namespace VKEngine{
	struct QueueFamilyIndice{
		optional<uint32_t> graphics;
		optional<uint32_t> transfer;
		optional<uint32_t> compute;
		optional<uint32_t> present;
		bool isSupport(VkQueueFlags flags) ;
	};
}

#endif