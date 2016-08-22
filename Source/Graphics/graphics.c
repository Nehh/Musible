#include "../Config/config.h"
#include "graphics.h"

int CreateInstance(GI *);
int LoadDefaultPhysicalDevice(GI *);
int GetPhysicalDeviceProperty(GI *);

int PrepareSwapChain(GI *);
	int CreateLogicalDevice(GI *);
int PrepareCommandPool(GI *);
int PrepareBuffers(GI *);
int PrepareDepth(GI *);
int PrepareRenderPass(GI *);
int PreparePipeline(GI *);
int PrepareFrameBuffers(GI *);

int CreateWindow()
{
	glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(*(int *)CONFIG[CFG_GAME_WIDTH], *(int *)CONFIG[CFG_GAME_HEIGHT], (char *)CONFIG[CFG_TITLE], NULL, NULL);
    
    return 0;
}

int StartRenderLoop()
{
	while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    return 0;
}

int CreateGraphicEnvironment()
{
	CreateInstance(&MW);
	LoadDefaultPhysicalDevice(&MW);
	GetPhysicalDeviceProperty(&MW);
	CreateWindow();
	
    PrepareSwapChain(&MW);
	PrepareCommandPool(&MW);
	/*PrepareBuffers(&MW);
	PrepareDepth(&MW);
	PrepareRenderPass(&MW);
	PreparePipeline(&MW);
	PrepareFrameBuffers(&MW);
	*/
	
	StartRenderLoop();
	
	return 0;
}

int CleanGraphics()
{
	vkFreeCommandBuffers(MW.device, MW.commandPool, 1, &MW.drawCommand);
    vkDestroyCommandPool(MW.device, MW.commandPool, NULL);
	
	vkDestroyDevice(MW.device, NULL);
    /*if (demo->validate) {
        demo->DestroyDebugReportCallback(demo->inst, demo->msg_callback, NULL);
    }*/
	vkDestroySurfaceKHR(MW.instance, MW.surface, NULL);
	vkDestroyInstance(MW.instance, NULL);
	glfwDestroyWindow(window);
    glfwTerminate();
	
	return 0;
}

int CreateInstance(GI * vk)
{
	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = u8"Musible",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = u8"Mine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};
	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = glfwExtensionCount,
		.ppEnabledExtensionNames = glfwExtensions,
	};
	
	VkResult result = vkCreateInstance(&createInfo, NULL, &vk->instance);
	
	return 0;
}

/** 
 * 用于载入显示设备的函数
 * 流程:
 * 先枚举所有可用的物理设备
 * 得到设备数量以后分配对应数量的内存来储存设备信息
 * 判断最佳设备
 * 采用这个设备
 * 释放内存
 */
int LoadDefaultPhysicalDevice(GI * vk)
{
	VkPhysicalDevice * physicalDevices = NULL;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vk->instance, &deviceCount, NULL);
	if (deviceCount == 0) return -1;
	physicalDevices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(vk->instance, &deviceCount, physicalDevices);
	vk->gpu = physicalDevices[0];
	free(physicalDevices);
	return 0;
}

int GetPhysicalDeviceProperty(GI * vk)
{
	vkGetPhysicalDeviceProperties(vk->gpu, &vk->gpuProps);

    /* Call with NULL data to get count */
    vkGetPhysicalDeviceQueueFamilyProperties(vk->gpu, &vk->queueCount, NULL);
	
	vk->queueProps = (VkQueueFamilyProperties *)malloc(vk->queueCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(vk->gpu, &vk->queueCount, vk->queueProps);
	vkGetPhysicalDeviceFeatures(vk->gpu, &vk->gpuFeatures);
    return 0;
}

int PrepareSwapChain(GI * vk)
{
	uint32_t index;
	if (glfwCreateWindowSurface(vk->instance, window, NULL, &vk->surface) != VK_SUCCESS) return -1;	//创建表面
	VkBool32 *supportsPresent = (VkBool32 *)malloc(vk->queueCount * sizeof(VkBool32));				//创建一个用来保存当前物理设备的表面是否支持KHR的数组
	
	for (index = 0; index < vk->queueCount; index++) {
		vk->fpGetPhysicalDeviceSurfaceSupportKHR(vk->gpu, index, vk->surface, &supportsPresent[index]);	//物理设备是否支持KHR?
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;	//图形队列节点索引
	uint32_t presentQueueNodeIndex = UINT32_MAX;	//当前队列节点索引
	for (index = 0; index < vk->queueCount; index++) {
		if ((vk->queueProps[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{	//如果
			if (graphicsQueueNodeIndex == UINT32_MAX) {
				graphicsQueueNodeIndex = index;
			}

			if (supportsPresent[index] == VK_TRUE) {
				graphicsQueueNodeIndex = index;
				presentQueueNodeIndex = index;
				break;
			}
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (uint32_t i = 0; i < vk->queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	free(supportsPresent);

	// Generate error if could not find both a graphics and a present queue
	if (graphicsQueueNodeIndex == UINT32_MAX ||
		presentQueueNodeIndex == UINT32_MAX) {
		return -1;
	}
	vk->graphicsQueueNodeIndex = graphicsQueueNodeIndex;
	
	CreateLogicalDevice(vk);
	
	vkGetDeviceQueue(vk->device, vk->graphicsQueueNodeIndex, 0,
                     &vk->queue);

    // Get the list of VkFormat's that are supported:
	uint32_t formatCount;
	if(vk->fpGetPhysicalDeviceSurfaceFormatsKHR(vk->gpu, vk->surface, &formatCount, NULL)) return -1;
	
    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	
    if(vk->fpGetPhysicalDeviceSurfaceFormatsKHR(vk->gpu, vk->surface,  &formatCount, surfFormats)) return -1;
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
        vk->format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        if(formatCount >= 1) return -1;
        vk->format = surfFormats[0].format;
    }
    vk->colorSpace = surfFormats[0].colorSpace;

    //vk->quit = false;
    //vk->curFrame = 0;

    // Get Memory information and properties
    vkGetPhysicalDeviceMemoryProperties(vk->gpu, &vk->memoryProperties);
	return 0;
}

int CreateLogicalDevice(GI * vk)
{
	float queue_priorities[1] = {0.0};
	uint32_t count;
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	/*
	//QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	vkGetPhysicalDeviceQueueFamilyProperties(vk->gpu, &vk->queueCount,
                                             NULL);
	if(vk->queueCount==0)return -1;
	vk->queueProps = (VkQueueFamilyProperties *)malloc(vk->queueCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(vk->gpu, &vk->queueCount, vk->queueProps);
	*/
    const VkDeviceQueueCreateInfo queue = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .queueFamilyIndex = vk->graphicsQueueNodeIndex,//indices.graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = queue_priorities,
	};
	VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = count,
        .ppEnabledExtensionNames = extensions,
        .pEnabledFeatures =
            NULL, // If specific features are required, pass them in here
    };
	VkPhysicalDeviceFeatures deviceFeatures = {};
	if (vkCreateDevice(vk->gpu, &createInfo, NULL, &vk->device) != VK_SUCCESS) return -1;
	return 0;
}

int PrepareCommandPool(GI * vk)
{
	const VkCommandPoolCreateInfo commandPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .queueFamilyIndex = vk->graphicsQueueNodeIndex,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    };
    vkCreateCommandPool(vk->device, &commandPoolInfo, NULL, &vk->commandPool);

    const VkCommandBufferAllocateInfo cmdInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = vk->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
	
	vkAllocateCommandBuffers(vk->device, &cmdInfo, &vk->drawCommand);
	return 0;
}
/**
int PrepareBuffersGI * vk)
int PrepareDepth(GI * vk);
int PrepareRenderPass(GI * vk);
int PreparePipeline(GI * vk);
int PrepareFrameBuffers(GI * vk);
*/
