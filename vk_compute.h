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

class Kernel{
	private :
	Context *context = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	public : 
	struct{
		VkDescriptorSetLayout layout;
		VkDescriptorSet set;
	}descriptors;
	VkPipelineLayout layout;
	VkPipeline pipeline;
	VkShaderModule module;
	string file_path;

	public : 
	Kernel();
	Kernel(Context *_context, const string _file_path);
	~Kernel();
	void create(Context *_context, const string _file_path);
	void build(VkPipelineCache cache);
	void loadShaderModule();
	void destroyShaderModule();
	void setupDescriptorSet();
	void setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> &bindings);
	void updateDescriptors();
	void destroy();
};
}
#endif