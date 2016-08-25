#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct {
	VkImage image;
	VkCommandBuffer cmd;
	VkImageView view;
} SwapchainBuffers;
	
typedef struct {
	VkInstance							instance;
	VkPhysicalDevice					gpu;
	VkPhysicalDeviceFeatures			gpuFeatures;
    VkPhysicalDeviceProperties			gpuProps;
	VkSurfaceKHR						surface;
	
	unsigned int						extensionCount;
	const char**						extensions;
	
	unsigned int						enabledDeviceExtensionCount;
	const char**						deviceExtensions;
	
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
	
	VkSwapchainKHR						swapchain;
	SwapchainBuffers					*buffers;
	uint32_t							swapchainImageCount;
	uint32_t							currentBuffer;
	
	/** 我们暂时还用不到Depth这个用来创建三维空间深度的东西
	struct {
        VkFormat format;

        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depth;
    **/
	
	VkCommandPool						commandPool;
	VkCommandBuffer						setupCommand;	// Command Buffer for initialization commands
    VkCommandBuffer						drawCommand;	// Command Buffer for drawing commands
}GI;	//Graphic Interface

GI MW;	//Main Window
