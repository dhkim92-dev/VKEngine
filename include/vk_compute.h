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
};

typedef struct _KernelArgs{
	uint32_t binding_idx;
	VkDescriptorType type;
	VkDescriptorBufferInfo *buffer_info = nullptr;
	VkDescriptorImageInfo *image_info;// = nullptr;
}KernelArgs;

class Kernel{
	private :
	Context *context = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	public : 
	struct Layouts{
		VkDescriptorSetLayout descriptor = VK_NULL_HANDLE;
		VkPipelineLayout pipeline = VK_NULL_HANDLE;
	}layouts;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkShaderModule module = VK_NULL_HANDLE;
	string file_path;

	public : 
	Kernel();
	Kernel(Context *_context, const string _file_path);
	~Kernel();
	void create(Context *_context, const string _file_path);
	void build(VkPipelineCache cache, VkSpecializationInfo *info=nullptr);
	void loadShaderModule();
	void destroyShaderModule();
	VkResult setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> bindings);
	VkResult allocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSet *set, uint32_t nr_descriptor_set);
	void setKernelArgs(VkDescriptorSet set, vector<KernelArgs> args);
	void destroy();
};
}
#endif