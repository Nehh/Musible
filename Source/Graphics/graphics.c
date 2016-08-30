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

int PrepareDescriptorLayout(GI *);
int PrepareRenderPass(GI *);
int PreparePipeline(GI *);
int PrepareFrameBuffers(GI *);

int CreatePipeline(GI *);

int CreateWindow(GI * vk)
{
	glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(*(int *)CONFIG[CFG_GAME_WIDTH], *(int *)CONFIG[CFG_GAME_HEIGHT], (char *)CONFIG[CFG_TITLE], NULL, NULL);
    
    return 0;
}

int StartRenderLoop(GI * vk)
{
	while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    CleanGraphics(vk);
    return 0;
}

int CreateGraphicEnvironment(GI * vk)
{
	CreateWindow(vk);
	CreateInstance(vk);	
	
	LoadDefaultPhysicalDevice(vk);
	GetPhysicalDeviceProperty(vk);
	GetDeviceExtensions(vk);
	
	CreateSurface(vk);
    PrepareSwapChain(vk);
	PrepareCommandPool(vk);
	PrepareBuffers(vk);
	PrepareDescriptorLayout(vk);
	PrepareRenderPass(vk);
	CreatePipeline(vk);
	/*PrepareDepth(&MW);
	PrepareRenderPass(&MW);
	PreparePipeline(&MW);
	PrepareFrameBuffers(&MW);
	*/
	
	StartRenderLoop(vk);
	
	return 0;
}

int CleanGraphics(GI * vk)
{
	int i;
	vkFreeCommandBuffers(vk->device, vk->commandPool, 1, &vk->drawCommand);
    vkDestroyCommandPool(vk->device, vk->commandPool, NULL);
	/*
	vkDestroyPipeline(vk->device, vk->pipeline, NULL);
    vkDestroyRenderPass(vk->device, vk->renderPass, NULL);
    vkDestroyPipelineLayout(vk->device, vk->pipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(vk->device, vk->descriptorLayout, NULL);

    vkDestroyBuffer(vk->device, vk->vertices.buf, NULL);
    vkFreeMemory(vk->device, vk->vertices.mem, NULL);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        vkDestroyImageView(vk->device, vk->textures[i].view, NULL);
        vkDestroyImage(vk->device, vk->textures[i].image, NULL);
        vkFreeMemory(vk->device, vk->textures[i].mem, NULL);
        vkDestroySampler(vk->device, vk->textures[i].sampler, NULL);
    }
*/
    for (i = 0; i < vk->swapchainImageCount; i++) {
        vkDestroyImageView(vk->device, vk->buffers[i].view, NULL);
    }
    
    vkDestroyRenderPass(vk->device, vk->renderPass, NULL);

    vkDestroySwapchainKHR(vk->device, vk->swapchain, NULL);
    free(vk->buffers);
	
	vkDestroyDevice(vk->device, NULL);
    /*if (demo->validate) {
        demo->DestroyDebugReportCallback(demo->inst, demo->msg_callback, NULL);
    }*/
	vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
	vkDestroyInstance(vk->instance, NULL);
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
        if(!(formatCount >= 1)) return -1;
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
        .clipped = VK_TRUE,
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
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = size,
		.pCode = code,
	};
	
	if(vkCreateShaderModule(vk->device, &createInfo, NULL, shaderModule) != VK_SUCCESS) {
		return -1;
	}
	
	return 0;
}

int PrepareDescriptorLayout(GI * vk) {
	const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
		.pushConstantRangeCount = 0, // Optional
		.pPushConstantRanges = 0, // Optional
	};
	if(vkCreatePipelineLayout(vk->device, &pPipelineLayoutCreateInfo, NULL, &vk->pipelineLayout) != VK_SUCCESS) return -1;
	return 0;
}

int PrepareRenderPass(GI * vk) {
    const VkAttachmentDescription attachments[1] = {
            [0] =
                {
                 .format = vk->format,
                 .samples = VK_SAMPLE_COUNT_1_BIT,
                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                 .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                 .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                },
    };
    const VkAttachmentReference color_reference = {
        .attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .flags = 0,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = NULL,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL,
    };
    const VkRenderPassCreateInfo rp_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
        .pDependencies = NULL,
    };
	
    if(!(vkCreateRenderPass(vk->device, &rp_info, NULL, &vk->renderPass)))return -1;
    return 0;
}

int CreatePipeline(GI * vk) {

    // Two stages: vs and fs
    void * vertShaderCode = NULL, * fragShaderCode = NULL;
    VkShaderModule vertShaderModule, fragShaderModule;
	int vertShaderSize, fragShaderSize;
	
	VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
	
	VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pDynamicStates = dynamicStateEnables,
	};
	
	LoadFileIntoMemory("Shaders/vert.spv", &vertShaderCode, &vertShaderSize);
	LoadFileIntoMemory("Shaders/frag.spv", &fragShaderCode, &fragShaderSize);
	CreateShaderModule(vk, vertShaderCode, &vertShaderModule, vertShaderSize);
	CreateShaderModule(vk, vertShaderCode, &fragShaderModule, fragShaderSize);

    //pipeline.stageCount = 2;
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
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = NULL, // Optional
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = NULL,
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};
	//这里可能需要改动 viewport 在tri.c里面实现方式不一样。

	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};
	
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_LINE_WIDTH;
	
	VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
	};
	
	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable = VK_FALSE,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = NULL, /// Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE,
	};
	
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
		.colorBlendOp = VK_BLEND_OP_ADD, // Optional
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
		.alphaBlendOp = VK_BLEND_OP_ADD, // Optional
	};
	
	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY, // Optional
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants[0] = 0.0f, // Optional
		.blendConstants[1] = 0.0f, // Optional
		.blendConstants[2] = 0.0f, // Optional
		.blendConstants[3] = 0.0f, // Optional
	};
	
	VkGraphicsPipelineCreateInfo pipeline = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.layout = vk->pipelineLayout,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pRasterizationState = &rasterizer,
		.pColorBlendState = &colorBlending,
		.pMultisampleState = &multisampling,
		.pViewportState = &viewportState,
		.pDepthStencilState = NULL,
		.stageCount = 2,
		.pStages = shaderStages,
		.renderPass = vk->renderPass,
		.pDynamicState = &dynamicState,
	};
	
	if(vkCreateGraphicsPipelines(vk->device, vk->pipelineCache, 1, &pipeline, NULL, &vk->pipeline)!=VK_SUCCESS) return -1;//fuck you
	
	vkDestroyShaderModule(vk->device, vertShaderModule, NULL);
    vkDestroyShaderModule(vk->device, fragShaderModule, NULL);
    return 0;
}

/*
int PrepareRenderPass(GI * vk);
int PreparePipeline(GI * vk);
int PrepareFrameBuffers(GI * vk);
*/
