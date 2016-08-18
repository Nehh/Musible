#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct {
	VkInstance						instance;
	VkPhysicalDevice				gpu;
	VkPhysicalDeviceFeatures		gpuFeatures;
	VKSurfaceKHR					surface;
	VkDevice						device;
	VkQueueFamilyProperties			*queueProps;
	uint32_t						graphics_queue_node_index;
	uint32_t						queueCount;
}GI;	//Graphic Interface

GI MW;	//Main Window
