#include "vulkan_app_base.hpp"

#include <vector>
#include <string>
#include <array>
#include <sstream>

namespace app
{

#define GetInstanceProcAddr(FuncName) \
	m_##FuncName = reinterpret_cast<PFN_##FuncName>(vkGetInstanceProcAddr(m_instance, #FuncName))

	static VkBool32 VKAPI_CALL DebugReportCallback
	(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectTypes,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData
	)
	{
		VkBool32 result = VK_FALSE;
		if (
			flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT ||
			flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT
			)
		{
			result = VK_TRUE;
		}

		std::stringstream ss;

		if (pLayerPrefix)
		{
			ss << "[" << pLayerPrefix << "]";
		}

		ss << pMessage << std::endl;

		OutputDebugStringA(ss.str().c_str());

		return result;
	}

	VulkanAppBase::VulkanAppBase()
	{
	}

	void VulkanAppBase::initialize(GLFWwindow* window, const char* appName)
	{
		initializeInstance(appName);

		selectPhysicalDevice();
		m_graphicsQueueIndex = searchGraphicsQueueIndex();

#ifdef _DEBUG
		enableDebugReport();
#endif // _DEBUG


		createDevice();

		prepareCommandPool();

		glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
		selectSurfaceFormat( VK_FORMAT_B8G8R8A8_UNORM );
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &m_surfaceCapabilities);
		VkBool32 isSuppoort = 0u;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_graphicsQueueIndex, m_surface, &isSuppoort);

		createSwapchain(window);

		createDepthBuffer();

		createViews();

		createRenderPass();

		createFramebuffer();

		prepareCommandBuffers();
		prepareFences();
		prepareSemaphores();

		prepare();
	}

	void VulkanAppBase::terminate()
	{
		vkDeviceWaitIdle(m_device);

		cleanup();

		vkFreeCommandBuffers(m_device, m_commandPool, m_commandBuffers.size(), m_commandBuffers.data());
		m_commandBuffers.clear();
		m_commandBuffers.shrink_to_fit();

		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		for (auto& frameBuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(m_device, frameBuffer, nullptr);
		}
		m_framebuffers.clear();
		m_framebuffers.shrink_to_fit();

		vkFreeMemory(m_device, m_depthBufferMemory, nullptr);
		vkDestroyImage(m_device, m_depthImage, nullptr);
		vkDestroyImageView(m_device, m_depthImageView, nullptr);

		for (auto& swapChainImageView : m_swapchainImageViews)
		{
			vkDestroyImageView(m_device, swapChainImageView, nullptr);
		}
		m_swapchainImageViews.clear();
		m_swapchainImageViews.shrink_to_fit();

		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

		for (auto& fence : m_fences)
		{
			vkDestroyFence(m_device, fence, nullptr);
		}
		m_fences.clear();
		m_fences.shrink_to_fit();

		vkDestroySemaphore(m_device, m_presentCompletedSemaphore, nullptr);
		vkDestroySemaphore(m_device, m_renderCompletedSemaphore, nullptr);

		vkDestroyCommandPool(m_device, m_commandPool, nullptr);

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

		vkDestroyDevice(m_device, nullptr);

#ifdef _DEBUG
		disableDebugReport();
#endif // _DEBUG

		vkDestroyInstance(m_instance, nullptr);
	}

	void VulkanAppBase::render()
	{

		uint32_t nextImageIndex = 0;
		vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_presentCompletedSemaphore, VK_NULL_HANDLE, &nextImageIndex);

		auto fence = m_fences[nextImageIndex];
		vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);

		std::array<VkClearValue, 2> clearValue =
		{ {
			{0.5f, 0.25f,0.25f,0.0f},
			{1.0f, 0}
		} };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = m_framebuffers[nextImageIndex];
		renderPassBeginInfo.renderArea.offset = VkOffset2D{ 0,0 };
		renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
		renderPassBeginInfo.pClearValues = clearValue.data();
		renderPassBeginInfo.clearValueCount = clearValue.size();

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		auto& commandBuffer = m_commandBuffers[nextImageIndex];
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_imageIndex = nextImageIndex;
		makeCommand(commandBuffer);

		vkCmdEndRenderPass(commandBuffer);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.pWaitDstStageMask = &waitStageMask;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_presentCompletedSemaphore;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_renderCompletedSemaphore;
		vkResetFences(m_device, 1, &fence);
		vkQueueSubmit(m_deviceQueue, 1, &submitInfo, fence);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &nextImageIndex;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_renderCompletedSemaphore;
		vkQueuePresentKHR(m_deviceQueue, &presentInfo);
	}

	void VulkanAppBase::checkResult( VkResult result )
	{
		if (result == VK_SUCCESS) return;
		DebugBreak();
	}

	void VulkanAppBase::initializeInstance(const char* appName)
	{
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = appName;
		applicationInfo.pEngineName = appName;
		applicationInfo.applicationVersion = VK_API_VERSION_1_1;
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

		std::vector< const char* >extensions;
		std::vector<VkExtensionProperties> extensionProperties;
		{
			uint32_t count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
			extensionProperties.resize(count);
			vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data());

			for (const auto& extensionProperty : extensionProperties)
			{
				extensions.push_back(extensionProperty.extensionName);
			}
		}

		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties>layerProperties(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

		std::vector<const char*>layerNames;
		layerNames.reserve(layerCount);

		for (auto& layerProperty : layerProperties)
		{
			layerNames.push_back(layerProperty.layerName);
		}

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.enabledLayerCount = layerNames.size();
		instanceCreateInfo.ppEnabledLayerNames = layerNames.data();
		instanceCreateInfo.enabledExtensionCount = extensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		auto result =  vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
		checkResult(result);

	}

	void VulkanAppBase::selectPhysicalDevice()
	{
		uint32_t  devCount = 0;
		auto result = vkEnumeratePhysicalDevices(m_instance, &devCount, nullptr);
		checkResult(result);
		std::vector< VkPhysicalDevice > physicalDevices(devCount);
		result = vkEnumeratePhysicalDevices(m_instance, &devCount, physicalDevices.data());
		checkResult(result);

		m_physicalDevice = physicalDevices[0];
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);
		
	}

	uint32_t VulkanAppBase::searchGraphicsQueueIndex()
	{
		uint32_t propertyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &propertyCount, nullptr);
		std::vector< VkQueueFamilyProperties > queueFamilyProperties(propertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &propertyCount, queueFamilyProperties.data());

		auto graphicsQueue = ~0u;

		for (auto i = 0u; i < propertyCount; ++i)
		{
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsQueue = i;
				break;
			}
		}

		return graphicsQueue;
	}

	void VulkanAppBase::createDevice()
	{
		const auto defaultQueuePriority = 1.0f;
		VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.queueFamilyIndex = m_graphicsQueueIndex;
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.pQueuePriorities = &defaultQueuePriority;

		std::vector<VkExtensionProperties> deviceExtensionsPropeties;

		{
			uint32_t count = 0u;
			auto result = vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &count, nullptr);
			checkResult(result);
			deviceExtensionsPropeties.resize(count);
			result = vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &count, deviceExtensionsPropeties.data());
			checkResult(result);
		}


		std::vector<const char*> extensions;

		//NOTE:これないとswapchainの生成ができない
		extensions.emplace_back("VK_KHR_swapchain");

		//for (const auto& deviceExtensionsProperty : deviceExtensionsPropeties)
		//{
		//	if (std::string(deviceExtensionsProperty.extensionName) == "VK_NV_acquire_winrt_display")continue;
		//	if (std::string(deviceExtensionsProperty.extensionName) == "VK_EXT_buffer_device_address")continue;
		//	extensions.push_back(deviceExtensionsProperty.extensionName);
		//}

		VkPhysicalDeviceFeatures physicalDeviceFeatures{};
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &physicalDeviceFeatures);

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		//NOTE:ppEnabledExtensionNamesが怪しい
		//     拡張機能によってはVkPhysicalDeviceProperties2が必要かも？
		deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
		deviceCreateInfo.enabledExtensionCount = extensions.size();
		deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

		auto result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
		checkResult(result);

		vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_deviceQueue);
	}

	void VulkanAppBase::prepareCommandPool()
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.queueFamilyIndex = m_graphicsQueueIndex;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		auto result = vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool);
		checkResult(result);
	}

	void VulkanAppBase::selectSurfaceFormat(VkFormat format)
	{
		uint32_t surfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &surfaceFormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &surfaceFormatCount, surfaceFormats.data());

		for (const auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format != format)continue;
			m_surfaceFormat = surfaceFormat;
		}
	}

	void VulkanAppBase::createSwapchain(GLFWwindow* window)
	{
		auto imageCount = std::max(2u, m_surfaceCapabilities.minImageCount);
		auto extent = m_surfaceCapabilities.currentExtent;
		if (extent.width == ~0u)
		{
			auto width = 0;
			auto height = 0;
			glfwGetWindowSize(window, &width, &height);
			extent.width = static_cast<uint32_t>(width);
			extent.height = static_cast<uint32_t>(height);
		}

		uint32_t queueFamilyIndices[] = { m_graphicsQueueIndex };
		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = m_surface;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = m_surfaceFormat.format;
		swapchainCreateInfo.imageColorSpace = m_surfaceFormat.colorSpace;
		swapchainCreateInfo.imageExtent = extent;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.preTransform = m_surfaceCapabilities.currentTransform;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.presentMode = m_presentMode;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		auto result = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
		checkResult(result);
		m_swapchainExtent = extent;
	}

	void VulkanAppBase::createDepthBuffer()
	{
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
		imageCreateInfo.extent.width = m_swapchainExtent.width;
		imageCreateInfo.extent.height = m_swapchainExtent.height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.arrayLayers = 1;
		auto result = vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_depthImage);
		checkResult(result);

		VkMemoryRequirements memoryRequirements{};
		vkGetImageMemoryRequirements(m_device, m_depthImage, &memoryRequirements);
		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_depthBufferMemory);
		vkBindImageMemory(m_device, m_depthImage, m_depthBufferMemory, 0);
	}

	uint32_t VulkanAppBase::getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestMemoryPropertyFlags) const
	{
		uint32_t memoryTypeIndex = 0u;
		for (uint32_t i = 0; i < m_physicalDeviceMemoryProperties.memoryTypeCount; ++i)
		{
			if (requestBits & 0b1)
			{
				const auto& types = m_physicalDeviceMemoryProperties.memoryTypes[i];
				if ((types.propertyFlags & requestMemoryPropertyFlags) == requestMemoryPropertyFlags)
				{
					memoryTypeIndex = i;
				}
			}
			requestBits >>= 1;
		}
		return memoryTypeIndex;
	}

	void VulkanAppBase::createViews()
	{
		createSwapchainViews();
		createDepthBufferBiews();
	}

	void VulkanAppBase::createSwapchainViews()
	{
		uint32_t imageCount = 0;
		auto result = vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
		checkResult(result);
		m_swapchainImages.resize(imageCount);
		result = vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
		checkResult(result);
		m_swapchainImageViews.resize(imageCount);
		for (auto i = 0u; i < imageCount; ++i)
		{
			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.image = m_swapchainImages[i];
			imageViewCreateInfo.format = m_surfaceFormat.format;
			imageViewCreateInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A,
			};
			imageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };
			auto result = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_swapchainImageViews[i]);
			checkResult(result);
		}
	}

	void VulkanAppBase::createDepthBufferBiews()
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.image = m_depthImage;
		imageViewCreateInfo.format = VK_FORMAT_D32_SFLOAT;
		imageViewCreateInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A,
		};

		imageViewCreateInfo.subresourceRange = {
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			0,1,0,1
		};

		auto result = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_depthImageView);
		checkResult(result);
	}

	void VulkanAppBase::createRenderPass()
	{
		std::array<VkAttachmentDescription, 2> attachments{};
		auto& colorTarget = attachments[0];
		auto& depthTarget = attachments[1];

		colorTarget.format = m_surfaceFormat.format;
		colorTarget.samples = VK_SAMPLE_COUNT_1_BIT;
		colorTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorTarget.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


		depthTarget.format = VK_FORMAT_D32_SFLOAT;
		depthTarget.samples = VK_SAMPLE_COUNT_1_BIT;
		depthTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthTarget.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference{}, depthReference{};

		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = attachments.size();
		renderPassCreateInfo.pAttachments = attachments.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		auto result = vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass);
		checkResult(result);
	}

	void VulkanAppBase::createFramebuffer()
	{
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_renderPass;
		framebufferCreateInfo.width = m_swapchainExtent.width;
		framebufferCreateInfo.height = m_swapchainExtent.height;
		framebufferCreateInfo.layers = 1;
		for (auto& swapchainImageView : m_swapchainImageViews)
		{
			std::array<VkImageView, 2>attachments;
			framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferCreateInfo.pAttachments = attachments.data();
			attachments[0] = swapchainImageView;
			attachments[1] = m_depthImageView;

			VkFramebuffer framebuffer;
			auto result = vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &framebuffer);
			checkResult(result);
			m_framebuffers.emplace_back(std::move(framebuffer));
		}
	}

	void VulkanAppBase::prepareCommandBuffers()
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = m_commandPool;
		commandBufferAllocateInfo.commandBufferCount = m_swapchainImageViews.size();
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		m_commandBuffers.resize(commandBufferAllocateInfo.commandBufferCount);
		auto result = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, m_commandBuffers.data());
		checkResult(result);
	}

	void VulkanAppBase::prepareFences()
	{
		m_fences.resize(m_swapchainImageViews.size());
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (auto& fence : m_fences)
		{
			auto result = vkCreateFence(m_device, &fenceCreateInfo, nullptr, &fence);
			checkResult(result);
		}
	}

	void VulkanAppBase::prepareSemaphores()
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderCompletedSemaphore);
		vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_presentCompletedSemaphore);
	}

	void VulkanAppBase::enableDebugReport()
	{
		GetInstanceProcAddr(vkCreateDebugReportCallbackEXT);
		GetInstanceProcAddr(vkDebugReportMessageEXT);
		GetInstanceProcAddr(vkDestroyDebugReportCallbackEXT);

		VkDebugReportFlagsEXT flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

		VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo{};
		debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugReportCallbackCreateInfo.flags = flags;
		debugReportCallbackCreateInfo.pfnCallback = &DebugReportCallback;
		m_vkCreateDebugReportCallbackEXT(m_instance, &debugReportCallbackCreateInfo, nullptr, &m_debugReportCallback);
	}

	void VulkanAppBase::disableDebugReport()
	{
		if (!m_vkDestroyDebugReportCallbackEXT)return;
		m_vkDestroyDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);
	}
}