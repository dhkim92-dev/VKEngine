#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <unistd.h>
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "vk_core.h"
#include "vk_application.h"
using namespace std;
using namespace VKEngine;


#define PROFILING(FPTR, FNAME) ({ \
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now(); \
		FPTR; \
		std::chrono::duration<double> t = std::chrono::system_clock::now() - start; \
		printf("%s operation time : %.4lf seconds\n",FNAME, t.count()); \
})

vector<const char *> getRequiredExtensions(  ){
	glfwInit();
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if(validationEnable) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	glfwTerminate();
	return extensions;	
}

unsigned char* loadImageFile(string file_path, int width, int height, int channels){
	int actual_comp = 0;
 	unsigned char *img_data=stbi_load(file_path.c_str(), &width, &height, &actual_comp, channels);

	printf("requested_comp : %d\n",channels);
	printf("actual_comp : %d\n", actual_comp);
	//float * data = new float[width*height*channels];
	//for(int i = 0 ; i < width * height * channels ; ++i){
	//	data[i] = (int)(img_data[i])/255.0;
	//}
	//free(img_data);
	if(!img_data){
		printf("failed to load image data\n");
		return nullptr;
	}

	for(uint32_t i = 0 ; i < 16 ; i++){
		uint32_t offset = i*4;
		printf("img_data[%d].rgba = %d %d %d %d\n", i, img_data[offset], img_data[offset+1], img_data[offset+2], img_data[offset+3]);
	}

	return img_data;
}

int main(int argc, const char *argv[])	
{
	vector<const char*> instance_extensions(getRequiredExtensions());
	vector<const char *> validations={"VK_LAYER_KHRONOS_validation"};
	vector<const char *> device_extensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	string app_name = "vulkan";
	string engine_name = "engine";
	unsigned char* img_data = loadImageFile("assets/image/lena.jpeg", 1200, 822, 4);
	LOG("image load completed\n");
	Engine engine(app_name, engine_name, instance_extensions, device_extensions, validations);
	engine.init();
	LOG("engine create done\n");
	Context context(&engine, 0, VK_QUEUE_COMPUTE_BIT, VK_NULL_HANDLE);
	LOG("context create done\n");
	CommandQueue queue(&context, VK_QUEUE_COMPUTE_BIT);
	LOG("queue create done\n");

	Kernel gaussian(&context, "shaders/compute/gaussian.comp.spv");
	VkDescriptorPool desc_pool;
	vector<VkDescriptorPoolSize> pool_sizes = {
		infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2),
	};
	VkDescriptorPoolCreateInfo descriptor_pool_CI = infos::descriptorPoolCreateInfo(
		static_cast<uint32_t>(pool_sizes.size()),
		pool_sizes.data(),
		2
	);
	VK_CHECK_RESULT(vkCreateDescriptorPool(VkDevice(context), &descriptor_pool_CI, nullptr, &desc_pool));

	gaussian.setupDescriptorSetLayout({
		infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
		infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
	});
	gaussian.allocateDescriptorSet(desc_pool);
	LOG("gaussian allocate descriptor\n");
	VkPipelineCache cache;
	VkPipelineCacheCreateInfo cache_CI={};
	cache_CI.sType=VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkCreatePipelineCache(VkDevice(context), &cache_CI, nullptr, &cache);
	gaussian.build(cache, nullptr);
	LOG("gaussian build done\n");

	Image d_input(&context);
	Image d_output(&context);

	VK_CHECK_RESULT(d_input.createImage(1200, 822, 1, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
						VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, 1, 1));
	VK_CHECK_RESULT(d_output.createImage(1200, 822, 1, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
						VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, 1, 1));

	VK_CHECK_RESULT(d_input.alloc( 1200*822*4, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ));
	VK_CHECK_RESULT(d_output.alloc( 1200*822*4, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ));
	VK_CHECK_RESULT(d_input.bind(0));
	VK_CHECK_RESULT(d_output.bind(0));
	VkImageSubresourceRange img_range;
	img_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	img_range.baseArrayLayer = 0;
	img_range.baseMipLevel = 0;
	img_range.layerCount = 1;
	img_range.levelCount = 1;

	VK_CHECK_RESULT(d_input.createImageView(VK_IMAGE_VIEW_TYPE_2D, img_range));
	VK_CHECK_RESULT(d_output.createImageView(VK_IMAGE_VIEW_TYPE_2D, img_range));

	VkSamplerCreateInfo sampler_CI = infos::samplerCreateInfo();
	sampler_CI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_CI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_CI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_CI.compareEnable = VK_TRUE;
	sampler_CI.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_CI.mipLodBias = 0.0f;
	sampler_CI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_CI.minLod = 0.0f;
	sampler_CI.maxLod = 1.0f;
	sampler_CI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_CI.maxAnisotropy = 1.0f;
	
	VK_CHECK_RESULT(d_input.createSampler(&sampler_CI));
	VK_CHECK_RESULT(d_output.createSampler(&sampler_CI));
	d_input.setupDescriptor();
	d_output.setupDescriptor();
	LOG("image setup done\n");

	Buffer staging(&context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
					1200 * 822 * 4, img_data);

	VkCommandBuffer copy_cmd = queue.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, true);
	VkBufferImageCopy region;
	
	region.bufferImageHeight = 822;
	region.bufferOffset = 0;
	region.bufferRowLength = 1200;
	region.imageExtent={1200,822,1};
	region.imageOffset = {0,0,0};
	region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1};

	d_input.setLayout(
		copy_cmd, VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
		VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
	queue.copyBufferToImage(copy_cmd, &staging, &d_input, &region);
	//vkCmdCopyBufferToImage(copy_cmd,
	//	VkBuffer(staging), VkImage(d_input), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//	1, &region);
	d_input.setLayout(
		copy_cmd, VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
		{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	d_output.setLayout(
		copy_cmd, VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
		{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	queue.endCommandBuffer(copy_cmd);
	queue.submit(&copy_cmd, 1, 0, nullptr, 0, nullptr, 0, nullptr);
	queue.waitIdle();
	LOG("submit done\n");
	staging.destroy();

	gaussian.setKernelArgs({
		{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, nullptr, &d_input.descriptor},
		{1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, nullptr, &d_output.descriptor}
	});
	queue.free(copy_cmd);
	
	Buffer h_output(&context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 1200*822*4, nullptr);

	VkCommandBuffer main_command = queue.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0, true);
	queue.bindKernel(main_command, &gaussian);
	d_input.setLayout(main_command, VK_IMAGE_ASPECT_COLOR_BIT, 
					VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
					{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, 
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_READ_BIT);

	queue.dispatch(main_command, 1200,822,1);
	
	d_output.setLayout(main_command, VK_IMAGE_ASPECT_COLOR_BIT, 
					VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
					{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, 
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
	queue.copyImageToBuffer(main_command, &d_output, &h_output, &region);
	d_output.setLayout(main_command, VK_IMAGE_ASPECT_COLOR_BIT, 
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
					{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, 
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
					VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT);

	queue.endCommandBuffer(main_command);
	VkFence fence = queue.createFence();
	queue.resetFences(&fence,1);
	queue.submit(&main_command, 1, 0, nullptr, 0, nullptr, 0, fence);
	queue.waitFences(&fence,1);
	queue.waitIdle();
	queue.free(main_command);
	queue.destroyFence(fence);

	//float *conv_data = new float[1200*822*3];
	unsigned char *cvt_data = new unsigned char[1200*822*4];
	h_output.copyTo(cvt_data, 1200*822*4);
	//h_output.destroy();
	for(uint32_t i = 0 ; i < 16 ; i++){
		uint32_t offset = i*4;
		printf("d_output[%d].rgba = %d %d %d %d\n", i, cvt_data[offset], cvt_data[offset+1], cvt_data[offset+2], cvt_data[offset+3]);
	}


	unsigned char *saved_data = new unsigned char[1200*822*3];
	stbi_write_jpg("test.jpeg", 1200, 822, 4, cvt_data, 100);
	delete [] saved_data;
	delete [] cvt_data;
	h_output.destroy();
	gaussian.destroy();
	d_input.destroy();
	d_output.destroy();
	vkDestroyDescriptorPool(VkDevice(context), desc_pool, nullptr);
	context.destroy();
	engine.destroy();

	return 0;
}
