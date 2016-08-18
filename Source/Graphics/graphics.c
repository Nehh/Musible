#include "../Config/config.h"
#include "graphics.h"

int CreateInstance(GI *);
int LoadDefaultPhysicalDevice(GI *);

int CreateWindow()
{
	glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(*(int *)CONFIG[CFG_GAME_WIDTH], *(int *)CONFIG[CFG_GAME_HEIGHT], (char *)CONFIG[CFG_TITLE], NULL, NULL);
	
	while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    
    return 0;
}

int CreateGraphicEnvironment()
{
	CreateInstance(&MW);
	LoadDefaultPhysicalDevice(&MW);
	return 0;
}

int CleanGraphics()
{
	vkDestroyDevice(MW.device, NULL);
    /*if (demo->validate) {
        demo->DestroyDebugReportCallback(demo->inst, demo->msg_callback, NULL);
    }*/
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
		.pEngineName = "Mine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};
	VkInstanceCreateInfo inst_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = glfwExtensionCount,
		.ppEnabledExtensionNames = glfwExtensions,
	};
	
	VkResult result = vkCreateInstance(&inst_info, NULL, &vk->instance);
	
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

int CreateSwapChain(GI * vk)
{
	 if (glfwCreateWindowSurface(vk->instance, window, NULL, &vk->surface) != VK_SUCCESS) return -1;
}

int CreateLogicalDevice(GI * vk)
{
	VkDevice device;
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
        .queueFamilyIndex = 0,//indices.graphicsFamily,
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
	if (vkCreateDevice(vk->gpu, &createInfo, NULL, &device) != VK_SUCCESS) return -1;
	return 0;
}
