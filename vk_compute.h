#ifndef __VK_COMPUTE_H__
#define __VK_COMPUTE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include "vk_context.h"
#include "vk_utils.h"
#include "vk_infos.h"

using namespace std;

namespace VKEngine{

struct KernelCreateInfo{
	string file_path;

	VkSpecializationInfo specialized_info;
};

struct Kernel{
	private :
	Context *context = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	public : 
	VkDescriptorSet descriptor_set;
	VkPipelineLayout layout;
	VkPipeline pipeline;
	VkShaderModule module;
	string file_path;

	public : 
	Kernel();
	Kernel(Context *_context, const string _file_path);
	~Kernel();
	void create(Context *_context, const string _file_path);
	void loadShaderModule();
	void destroyShaderModule();
	void setupDescriptorSet();
	void setupDescriptorSetLayout();
	void updateDescriptors();
	void destroy();
};

class ComputeProgram{
	private :
	VkDevice device = VK_NULL_HANDLE;
	Context *context = nullptr;
	VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
	unordered_map<string, Kernel> kernels;
	public :
	ComputeProgram();
	ComputeProgram(Context *_context);
	void createDescriptorPool();
	void attachKernel();
	void allocateDescriptorSet(string kernel_name);
	void build();
	void destroy();

	Kernel operator [](string kernel_name) {
		return kernels[kernel_name.c_str()];
	}
};

}
#endif