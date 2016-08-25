#include "../Config/config.h"
#include "graphics.h"

int CreateInstance(GI *);
int LoadDefaultPhysicalDevice(GI *);
int GetPhysicalDeviceProperty(GI *);
int GetDeviceExtensions(GI *);

int CreateSurface(GI *);

int PrepareSwapChain(GI *);
	int CreateLogicalDevice(GI *);
int PrepareCommandPool(GI *);
int PrepareBuffers(GI *);
int PrepareDepth(GI *);
	VkBool32 MemoryTypeFromProperties(GI *, uint32_t, VkFlags, uint32_t *);

int VkSetImageLayout(GI *, VkImage, VkImageAspectFlags, VkImageLayout, VkImageLayout, VkAccessFlagBits);
	
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
	CreateWindow();
	CreateInstance(&MW);	
	
	LoadDefaultPhysicalDevice(&MW);
	GetPhysicalDeviceProperty(&MW);
	GetDeviceExtensions(&MW);
	
	CreateSurface(&MW);
    PrepareSwapChain(&MW);
	PrepareCommandPool(&MW);
	PrepareBuffers(&MW);
	/*PrepareDepth(&MW);
	PrepareRenderPass(&MW);
	PreparePipeline(&MW);
	PrepareFrameBuffers(&MW);
	*/
	
	StartRenderLoop();
	
	return 0;
}

int CleanGraphics()
{
	int i;
	vkFreeCommandBuffers(MW.device, MW.commandPool, 1, &MW.drawCommand);
    vkDestroyCommandPool(MW.device, MW.commandPool, NULL);
	/*
	vkDestroyPipeline(MW.device, MW.pipeline, NULL);
    vkDestroyRenderPass(MW.device, MW.renderPass, NULL);
    vkDestroyPipelineLayout(MW.device, MW.pipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(MW.device, MW.descriptorLayout, NULL);

    vkDestroyBuffer(MW.device, MW.vertices.buf, NULL);
    vkFreeMemory(MW.device, MW.vertices.mem, NULL);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        vkDestroyImageView(MW.device, MW.textures[i].view, NULL);
        vkDestroyImage(MW.device, MW.textures[i].image, NULL);
        vkFreeMemory(MW.device, MW.textures[i].mem, NULL);
        vkDestroySampler(MW.device, MW.textures[i].sampler, NULL);
    }
*/
    for (i = 0; i < MW.swapchainImageCount; i++) {
        vkDestroyImageView(MW.device, MW.buffers[i].view, NULL);
    }

    vkDestroyImageView(MW.device, MW.depth.view, NULL);
    vkDestroyImage(MW.device, MW.depth.image, NULL);
    vkFreeMemory(MW.device, MW.depth.mem, NULL);

    vkDestroySwapchainKHR(MW.device, MW.swapchain, NULL);
    free(MW.buffers);
	
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
	vk->extensions = glfwGetRequiredInstanceExtensions(&vk->extensionCount);
	
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
		.enabledExtensionCount = vk->extensionCount,
		.ppEnabledExtensionNames = vk->extensions,
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
int CreateSurface(GI * vk)
{
	if (glfwCreateWindowSurface(vk->instance, window, NULL, &vk->surface) != VK_SUCCESS) return -1;
	return 0;
}

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

int GetDeviceExtensions(GI * vk)
{
	uint32_t device_extension_count = 0;
    vk->enabledDeviceExtensionCount = 0;
	
    vkEnumerateDeviceExtensionProperties(vk->gpu, NULL, &device_extension_count, NULL);
	vk->deviceExtensions = malloc(sizeof(const char **)*device_extension_count);
	
    if (device_extension_count > 0) {
        VkExtensionProperties *device_extensions = malloc(sizeof(VkExtensionProperties) * device_extension_count);
        vkEnumerateDeviceExtensionProperties(vk->gpu, NULL, &device_extension_count, device_extensions);

        for (uint32_t i = 0; i < device_extension_count; i++) {
            if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, device_extensions[i].extensionName)) {
                vk->deviceExtensions[vk->enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
            if(!(vk->enabledDeviceExtensionCount < 64))return -1;
        }

        free(device_extensions);
    }
    return 0;
}

/**
 * 为交换链创建支持
 * 获取物理设备信息
 * 获取渲染队列信息
 * 初始化部分信息
 */
int PrepareSwapChain(GI * vk)
{
	uint32_t index;
	
	VkBool32 *supportsPresent = (VkBool32 *)malloc(vk->queueCount * sizeof(VkBool32));				//创建一个用来保存当前物理设备的表面是否支持KHR的数组
	
	for (index = 0; index < vk->queueCount; index++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(vk->gpu, index, vk->surface, &supportsPresent[index]);	//物理设备是否支持KHR?
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
	if(vkGetPhysicalDeviceSurfaceFormatsKHR(vk->gpu, vk->surface, &formatCount, NULL)) return -1;
	
    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	
    if(vkGetPhysicalDeviceSurfaceFormatsKHR(vk->gpu, vk->surface,  &formatCount, surfFormats)) return -1;
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
	VkPhysicalDeviceFeatures features;
    memset(&features, 0, sizeof(features));
	VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = vk->enabledDeviceExtensionCount,
        .ppEnabledExtensionNames = (const char *const *)vk->deviceExtensions,
        .pEnabledFeatures =
            &features, // If specific features are required, pass them in here
    };
	//VkPhysicalDeviceFeatures deviceFeatures = {};
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

int PrepareBuffers(GI * vk)
{
    VkSwapchainKHR oldSwapchain = vk->swapchain;

    // Check the surface capabilities and formats
    VkSurfaceCapabilitiesKHR surfCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->gpu, vk->surface, &surfCapabilities);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk->gpu, vk->surface, &presentModeCount, NULL);
    VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk->gpu, vk->surface, &presentModeCount, presentModes);

    VkExtent2D swapchainExtent;
    // width and height are either both -1, or both not -1.
    if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchainExtent.width = vk->width;
        swapchainExtent.height = vk->height;
    } else {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCapabilities.currentExtent;
        vk->width = surfCapabilities.currentExtent.width;
        vk->height = surfCapabilities.currentExtent.height;
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // Determine the number of VkImage's to use in the swap chain (we desire to
    // own only 1 image at a time, besides the images being displayed and
    // queued for display):
    uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
    if ((surfCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount)) {
        // Application must settle for fewer images than desired:
        desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCapabilities.currentTransform;
    }

    const VkSwapchainCreateInfoKHR swapchain = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = vk->surface,
        .minImageCount = desiredNumberOfSwapchainImages,
        .imageFormat = vk->format,
        .imageColorSpace = vk->colorSpace,
        .imageExtent =
            {
             .width = swapchainExtent.width, .height = swapchainExtent.height,
            },
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = preTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .presentMode = swapchainPresentMode,
        .oldSwapchain = oldSwapchain,
        .clipped = true,
    };
    uint32_t i;

    vkCreateSwapchainKHR(vk->device, &swapchain, NULL, &vk->swapchain);

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vk->device, oldSwapchain, NULL);
    }

    vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swapchainImageCount, NULL);

    VkImage *swapchainImages = (VkImage *)malloc(vk->swapchainImageCount * sizeof(VkImage));
	vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swapchainImageCount, swapchainImages);

    vk->buffers = (SwapchainBuffers *)malloc(sizeof(SwapchainBuffers) * vk->swapchainImageCount);

    for (i = 0; i < vk->swapchainImageCount; i++) {
        VkImageViewCreateInfo color_attachment_view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .format = vk->format,
            .components =
                {
                 .r = VK_COMPONENT_SWIZZLE_R,
                 .g = VK_COMPONENT_SWIZZLE_G,
                 .b = VK_COMPONENT_SWIZZLE_B,
                 .a = VK_COMPONENT_SWIZZLE_A,
                },
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1},
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .flags = 0,
        };

        vk->buffers[i].image = swapchainImages[i];

        color_attachment_view.image = vk->buffers[i].image;

        vkCreateImageView(vk->device, &color_attachment_view, NULL, &vk->buffers[i].view);
    }

    vk->currentBuffer = 0;

    if (NULL != presentModes) {
        free(presentModes);
    }
    return 0;
}

int PrepareDepth(GI * vk)
{
	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
    const VkImageCreateInfo image = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depth_format,
        .extent = {vk->width, vk->height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .flags = 0,
    };
    VkMemoryAllocateInfo mem_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = 0,
        .memoryTypeIndex = 0,
    };
    VkImageViewCreateInfo view = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .image = VK_NULL_HANDLE,
        .format = depth_format,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1},
        .flags = 0,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
    };

    VkMemoryRequirements mem_reqs;

    vk->depth.format = depth_format;

    /* create image */
    vkCreateImage(vk->device, &image, NULL, &vk->depth.image);

    /* get memory requirements for this object */
    vkGetImageMemoryRequirements(vk->device, vk->depth.image, &mem_reqs);

    /* select memory size and type */
    mem_alloc.allocationSize = mem_reqs.size;
    MemoryTypeFromProperties(vk, mem_reqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex);

    /* allocate memory */
    vkAllocateMemory(vk->device, &mem_alloc, NULL, &vk->depth.mem);

    /* bind memory */
    vkBindImageMemory(vk->device, vk->depth.image, vk->depth.mem, 0);

    VkSetImageLayout(vk, vk->depth.image, VK_IMAGE_ASPECT_DEPTH_BIT,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0);

    /* create image view */
    view.image = vk->depth.image;
    vkCreateImageView(vk->device, &view, NULL, &vk->depth.view);
	return 0;
}

VkBool32 MemoryTypeFromProperties(GI * vk, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((vk->memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return VK_TRUE;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return VK_FALSE;
}

int VkSetImageLayout(GI						*vk,
					 VkImage				image,
                     VkImageAspectFlags		aspectMask,
                     VkImageLayout			old_image_layout,
                     VkImageLayout			new_image_layout,
                     VkAccessFlagBits		srcAccessMask) {

    if (vk->setupCommand == VK_NULL_HANDLE) {
        const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = vk->commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkAllocateCommandBuffers(vk->device, &cmd, &vk->setupCommand);

        VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = 0,
            .pInheritanceInfo = NULL,
        };
        vkBeginCommandBuffer(vk->setupCommand, &cmd_buf_info);
    }

    VkImageMemoryBarrier image_memory_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = 0,
        .oldLayout = old_image_layout,
        .newLayout = new_image_layout,
        .image = image,
        .subresourceRange = {aspectMask, 0, 1, 0, 1}};

    if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        /* Make sure anything that was copying from this image has completed */
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }

    if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        /* Make sure any Copy or CPU writes to image are flushed */
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    }

    VkImageMemoryBarrier *pmemory_barrier = &image_memory_barrier;

    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    vkCmdPipelineBarrier(vk->setupCommand, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);
	
	return 0;
}

int CreateShaderModule(GI * vk, const void * code, VkShaderModule * shaderModule, int size)
{
	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		.codeSize = size;
		.pCode = code;
	}
	
	if(vkCreateShaderModule(vk->device, &createInfo, NULL, shaderModule) != VK_SUCCESS) {
		return -1;
	}
	
	free(code);
	
	return 0;
}

static void CreatePipeline(GI * vk) {

    // Two stages: vs and fs
    void * vertShaderCode, fragShaderCode;
    VkShaderModule vertShaderModule, fragShaderModule;
	int vertShaderSize, fragShaderSize;
	LoadFileIntoMemory("shaders/vert.spv", vertShaderCode, &vertShaderSize);
	LoadFileIntoMemory("shaders/frag.spv", fragShaderCode, &fragShaderSize);
	CreateShaderModule(vk, vertShaderCode, &vertShaderModule, &vertShaderSize);
	CreateShaderModule(vk, vertShaderCode, &fragShaderModule, &fragShaderSize);

    pipeline.stageCount = 2;
    VkPipelineShaderStageCreateInfo shaderStages[2];
    memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName = "main";
    
    vkDestroyShaderModule(vk->device, fragShaderModule, NULL);
    vkDestroyShaderModule(vk->device, vertShaderModule, NULL);
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		.vertexBindingDescriptionCount = 0;
		.pVertexBindingDescriptions = NULL; // Optional
		.vertexAttributeDescriptionCount = 0;
		.pVertexAttributeDescriptions = NULL;
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		.primitiveRestartEnable = VK_FALSE;
	};
	VkViewport viewport = {
		.x = 0.0f;
		.y = 0.0f;
		.width = (float) swapChainExtent.width;
		.height = (float) swapChainExtent.height;
		.minDepth = 0.0f;
		.maxDepth = 1.0f;
	};
    return 0;
}

/*
int PrepareRenderPass(GI * vk);
int PreparePipeline(GI * vk);
int PrepareFrameBuffers(GI * vk);
*/
