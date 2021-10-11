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
	/*
	KernelArgs(uint32_t idx, VkDescriptorType t, VkDescriptorBufferInfo *bi, VkDescriptorImageInfo *ii){
		binding_idx = idx;
		type = t;
		buffer_info = bi;
		image_info = ii;
	}
	*/

}KernelArgs;

class Kernel{
	private :
	Context *context = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	public : 
	struct{
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
		VkDescriptorSet set = VK_NULL_HANDLE;
	}descriptors;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkShaderModule module = VK_NULL_HANDLE;
	string file_path;

	public : 
	Kernel();
	Kernel(Context *_context, const string _file_path);
	~Kernel();
	void create(Context *_context, const string _file_path);
	void build(VkPipelineCache cache);
	void build(VkPipelineCache cache, VkSpecializationInfo *info=nullptr);
	void loadShaderModule();
	void destroyShaderModule();
	void setupDescriptorSetLayout(vector<VkDescriptorSetLayoutBinding> bindings);
	void allocateDescriptorSet(VkDescriptorPool pool);
	void setKernelArgs(vector<KernelArgs> args);
	void destroy();
};
}
#endif