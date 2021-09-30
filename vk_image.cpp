#ifndef __VK_IMAGE_CPP__
#define __VK_IMAGE_CPP__

#include "vk_image.h"

namespace VKEngine{

Image::Image(){}
Image::Image(Context *ctx, 
			uint32_t h, uint32_t w, uint32_t ch, 
			VkImageUsageFlags img_usages, VkMemoryPropertyFlags mem_flags)
{
	context = ctx;
	height = h;
	width = w;
	usages = img_usages;
	mem_properties = mem_flags;
	create(ctx, height, width, usages, mem_properties);
}

void Image::create(
	Context *ctx, 
	uint32_t h, uint32_t w, uint32_t ch,
	VkImageUsageFlags usages, VkMemoryPropertyFlags mem_flags)
{
}

VkImage createImage(uint32_t height, uint32_t width, uint32_t channels)
{
	VkImageCreateInfo image_CI ;//= infos::imageCreateInfo()

}


}
#endif
