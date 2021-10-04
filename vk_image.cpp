#ifndef __VK_IMAGE_CPP__
#define __VK_IMAGE_CPP__

#include "vk_image.h"

namespace VKEngine{


Image::Image(){

}

Image::Image(Context *ctx){
	create(ctx);
}

Image::~Image(){
	destroy();
}

void Image::destroy(){
	if(image != VK_NULL_HANDLE){vkDestroyImage(device, image, nullptr);}
	if(view != VK_NULL_HANDLE){vkDestroyImageView(device, view, nullptr);}
	if(memory != VK_NULL_HANDLE){vkFreeMemory(device, memory, nullptr);}
	if(sampler != VK_NULL_HANDLE){vkDestroySampler(device, sampler, nullptr);}
}

void Image::create(Context *ctx){
	this->context = ctx;
	device = VkDevice(*ctx);
}

VkResult Image::createImage(uint32_t w, uint32_t h, uint32_t ch,
					VkImageType img_type, VkImageUsageFlags img_usage,
					VkFormat img_format, VkImageTiling img_tiling, 
					VkSampleCountFlagBits sample, uint32_t mip_level, uint32_t array_layer
					){
	VkImageCreateInfo info = infos::imageCreateInfo();
	info.imageType = img_type;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	info.usage = img_usage;
	info.format = img_format;
	info.tiling = img_tiling;
	info.samples = sample;
	info.extent = {w,h,ch};
	info.mipLevels = mip_level;
	info.arrayLayers = array_layer;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkResult res = vkCreateImage(device, &info, nullptr, &image);
	height = h;
	width = w;
	channels = ch;
	format = img_format;
	nr_samples = sample;
	return res;
}

VkResult Image::alloc(VkDeviceSize sz_mem, VkMemoryPropertyFlags flags){
	VkMemoryAllocateInfo info = infos::memoryAllocateInfo();
	VkBool32 found;
	vkGetImageMemoryRequirements(device, image, &memory_requirements);
	LOG("Image::alloc GetMemoryRequirements\n");
	info.memoryTypeIndex = context->getMemoryType(memory_requirements.memoryTypeBits, flags, &found);
	LOG("Image::alloc getMemoryType done\n");
	info.allocationSize = memory_requirements.size;
	sz_memory = sz_mem;
	memory_properties = flags;
	return vkAllocateMemory(device, &info, nullptr, &memory);
}


void Image::setMemory(VkDeviceMemory mem){
	memory = mem;
}

VkResult Image::bind(VkDeviceSize offset){
	return vkBindImageMemory(device, this->image, this->memory, offset);
}

VkResult Image::createImageView(VkImageViewType view_type, VkImageSubresourceRange range){
	VkImageViewCreateInfo view_CI = infos::imageViewCreateInfo();
	view_CI.image = image;
	view_CI.format = format;
	view_CI.viewType = view_type;
	view_CI.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	view_CI.subresourceRange = range;
	return vkCreateImageView(device, &view_CI, nullptr, &view);
}

VkResult Image::createSampler(VkSamplerCreateInfo *info){
	return vkCreateSampler(device, info, nullptr, &sampler);	
}

void Image::setupDescriptor(){
	descriptor.imageLayout = layout;
	descriptor.imageView = view;
	descriptor.sampler = sampler;
}

VkResult Image::map(VkDeviceSize offset, VkDeviceSize size){
	return vkMapMemory(device, memory, offset, size, 0, &data);
}

void Image::unmap(){
	vkUnmapMemory(device, memory);
}

VkResult Image::invalidate(VkDeviceSize offset, VkDeviceSize size){
	VkMappedMemoryRange range = {};
	range.memory = memory;
	range.offset = offset;
	range.size = size;
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	return vkInvalidateMappedMemoryRanges(device, 1, &range);
}

VkResult Image::flush(VkDeviceSize offset, VkDeviceSize size){
	VkMappedMemoryRange range = {};
	range.memory = memory;
	range.offset = offset;
	range.size = size;
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	return vkFlushMappedMemoryRanges(device, 1, &range);
}

void Image::copyFrom(void *src, VkDeviceSize size){
	map(0, memory_requirements.size);
	assert(data);
	memcpy(data, src, size);
	if(!(memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		flush(0, VK_WHOLE_SIZE);
	unmap();
}

void Image::copyTo(void *dst, VkDeviceSize size){
	map(0, VK_WHOLE_SIZE);
	assert(data);
	memcpy(dst, data, size);
	if(!(memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		invalidate(0, VK_WHOLE_SIZE);
	unmap();
}

void Image::setLayout(VkCommandBuffer command_buffer, VkImageAspectFlags aspect_mask,
					VkImageLayout old_layout, VkImageLayout new_layout, 
					VkImageSubresourceRange subresource_range,
					VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
					VkAccessFlags src_mask, VkAccessFlags dst_mask
					){
		
	VkImageMemoryBarrier barrier = infos::imageMemoryBarrier(old_layout, new_layout);
	barrier.image = this->image;
	barrier.srcAccessMask = src_mask;
	barrier.dstAccessMask = dst_mask;
	barrier.subresourceRange = subresource_range;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkCmdPipelineBarrier(command_buffer,
		src_stage, dst_stage, 
		0, 
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	layout = new_layout;
	setupDescriptor();
}


}
#endif
