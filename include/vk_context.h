#ifndef __VK_CONTEXT_H__
#define __VK_CONTEXT_H__

#include <vulkan/vulkan.h>
#include "vk_physicaldevice.h"
#include "vk_utils.h"
#include "vk_infos.h"
#include "vk_queue_family.h"

namespace VKEngine{
class Context{
// fields
private:
VkDevice device=VK_NULL_HANDLE;
PhysicalDevice *pdevice = nullptr;
VkQueueFlags queue_flags;
VkPhysicalDeviceFeatures device_features={};
QueueFamilyIndice indice;
public:

//methods
private:

vector<VkDeviceQueueCreateInfo> createQueueCI(float *priorities);
public:
Context();
Context(PhysicalDevice *pdevice, VkQueueFlags flags);
~Context();
void destroy();

// Vulkan Resources
VkResult initDevice();
VkResult createCommandPool(VkCommandPool *pool, VkQueueFlags type, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
VkResult createSemaphore(VkSemaphore *semaphore);
VkResult createEvent(VkEvent *event);
VkResult createFence(VkFence *fence);
VkResult useSurface(VkSurfaceKHR surface);

void destroyCommandPool(VkCommandPool *pool);
void destroySemaphore(VkSemaphore* semaphore);
void destroyEvent(VkEvent *event);
void destroyFence(VkFence *fence);

//getter
PhysicalDevice *getPhysicalDevice() const ;
VkDevice getDevice() const ;
QueueFamilyIndice getQueueFamily() const ;
//setter
void setQueueFlags(VkQueueFlags flags);
void setPhysicalDevice(PhysicalDevice *device);
void setDevice(VkDevice device);
void setDeviceFeatures(VkPhysicalDeviceFeatures features);


//utils


};
}
#endif