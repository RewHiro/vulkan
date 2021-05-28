#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

#pragma comment(lib, "vulkan-1.lib")

namespace app
{
	class VulkanAppBase
	{
	public:
		VulkanAppBase();
		virtual ~VulkanAppBase() = default;

		void initialize(GLFWwindow* window, const char* appName);
		void terminate();

		virtual void prepare();
		virtual void render();

	protected:

		static void checkResult(VkResult);

		void initializeInstance(const char* appName);

		void selectPhysicalDevice();

		uint32_t searchGraphicsQueueIndex();

		void createDevice();

		void prepareCommandPool();

		VkInstance m_instance = nullptr;

		VkPhysicalDevice m_physicalDevice = nullptr;
		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties{};

		VkDevice m_device = nullptr;
		VkQueue m_deviceQueue = nullptr;
		uint32_t m_graphicsQueueIndex = 0;

		VkCommandPool m_commandPool = 0ull;

	};
}