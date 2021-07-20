#ifndef __VK_QUEUE_FAMILY_CPP__
#define __VK_QUEUE_FAMILY_CPP__

#include "vk_queue_family.h"
using namespace std;

namespace VKEngine{
	bool QueueFamilyIndice::isSupport(VkQueueFlags flags, bool present_mode){
		bool ret = true;
		if(flags & VK_QUEUE_GRAPHICS_BIT) ret&=graphics.has_value();
		if(flags & VK_QUEUE_COMPUTE_BIT) ret&=compute.has_value();
		if(flags & VK_QUEUE_TRANSFER_BIT) ret&=transfer.has_value();
		ret &= present.has_value();
		return ret;
	}
};

#endif