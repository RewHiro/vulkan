#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

#pragma comment(lib, "vulkan-1.lib")

#include <vector>

namespace app
{
	class VulkanAppBase
	{
	public:
		VulkanAppBase();
		virtual ~VulkanAppBase() = default;

		void initialize(GLFWwindow* window, const char* appName);
		void terminate();

		virtual void prepare() {}
		virtual void cleanup() {}
		virtual void makeCommand(VkCommandBuffer command) {}

		virtual void render();

	protected:

		static void checkResult(VkResult);

		void initializeInstance(const char* appName);

		void selectPhysicalDevice();

		uint32_t searchGraphicsQueueIndex();

		void createDevice();

		void prepareCommandPool();

		void selectSurfaceFormat(VkFormat format);

		void createSwapchain(GLFWwindow* window);

		void createDepthBuffer();

		uint32_t getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestMemoryPropertyFlags) const;

		void createViews();
		void createSwapchainViews();
		void createDepthBufferBiews();

		void createRenderPass();
		void createFramebuffer();

		void prepareCommandBuffers();
		void prepareFences();
		void prepareSemaphores();

		void enableDebugReport();
		void disableDebugReport();

		//===================================================================================================

		VkInstance m_instance = nullptr;

		VkPhysicalDevice m_physicalDevice = nullptr;
		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties{};

		VkDevice m_device = nullptr;
		VkQueue m_deviceQueue = nullptr;
		uint32_t m_graphicsQueueIndex = 0;

		VkCommandPool m_commandPool = 0ull;

		VkSurfaceKHR m_surface = 0ull;
		VkSurfaceFormatKHR m_surfaceFormat{};
		VkSurfaceCapabilitiesKHR m_surfaceCapabilities{};
		VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkSwapchainKHR m_swapchain = 0ull;
		VkExtent2D m_swapchainExtent{};

		VkImage m_depthImage = 0ull;
		VkDeviceMemory m_depthBufferMemory = 0ull;
		VkImageView m_depthImageView = 0ull;

		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;

		VkRenderPass m_renderPass = 0ull;
		std::vector<VkFramebuffer> m_framebuffers;

		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkFence> m_fences;

		VkSemaphore m_renderCompletedSemaphore = 0ull;
		VkSemaphore m_presentCompletedSemaphore = 0ull;

		PFN_vkCreateDebugReportCallbackEXT m_vkCreateDebugReportCallbackEXT = nullptr;
		PFN_vkDebugReportMessageEXT m_vkDebugReportMessageEXT = nullptr;
		PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallbackEXT = nullptr;
		VkDebugReportCallbackEXT m_debugReportCallback = 0ull;

		uint32_t m_imageIndex = 0;
	};
}