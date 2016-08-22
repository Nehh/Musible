#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct {
	VkInstance							instance;
	VkPhysicalDevice					gpu;
	VkPhysicalDeviceFeatures			gpuFeatures;
    VkPhysicalDeviceProperties			gpuProps;
	VkSurfaceKHR						surface;
	
	int									width, height;
	VkFormat							format;
	VkColorSpaceKHR						colorSpace;
	
	VkFramebuffer						*framebuffers;

	VkPhysicalDeviceMemoryProperties	memoryProperties;
	
	VkDevice							device;
	VkQueue								queue;
	VkQueueFamilyProperties				*queueProps;
	uint32_t							graphicsQueueNodeIndex;
	uint32_t							queueCount;
	
	VkCommandPool						commandPool;
	VkCommandBuffer						setupCommand;	// Command Buffer for initialization commands
    VkCommandBuffer						drawCommand;	// Command Buffer for drawing commands
	
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR		fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR						fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR						fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR						fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR						fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR							fpQueuePresentKHR;
}GI;	//Graphic Interface

GI MW;	//Main Window
