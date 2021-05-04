#include "vulkan_app_base.hpp"

#include <vector>
#include <string>

namespace app
{
	VulkanAppBase::VulkanAppBase()
	{
	}

	void VulkanAppBase::initialize(GLFWwindow* window, const char* appName)
	{
	}

	void VulkanAppBase::terminate()
	{
	}
	void VulkanAppBase::prepare()
	{
	}
	void VulkanAppBase::render()
	{
	}

	void VulkanAppBase::initializeInstance(const char* appName)
	{
		std::vector< const char* >extensions;

		VkApplicationInfo vulkanApplicationInfo{};
		vulkanApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vulkanApplicationInfo.pApplicationName = appName;
		vulkanApplicationInfo.pEngineName = appName;
		vulkanApplicationInfo.applicationVersion = VK_API_VERSION_1_1;
		vulkanApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

		const char* layers[] = { "VK_LAYTER_LUNARG_standard_validation" };
		VkInstanceCreateInfo vulkanInstanceCreateInfo{};
		vulkanInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vulkanInstanceCreateInfo.enabledLayerCount = 1;
		vulkanInstanceCreateInfo.ppEnabledLayerNames = layers;
		vulkanInstanceCreateInfo.enabledExtensionCount = extensions.size();
		vulkanInstanceCreateInfo.ppEnabledExtensionNames = extensions.data();
		vulkanInstanceCreateInfo.pApplicationInfo = &vulkanApplicationInfo;
		auto result =  vkCreateInstance(&vulkanInstanceCreateInfo, nullptr, &m_instance);

	}

	void VulkanAppBase::selectPhysicalDevice()
	{
		auto devCount = 0u;
		vkEnumeratePhysicalDevices(m_instance, &devCount, nullptr);
		std::vector< VkPhysicalDevice > physicalDevices(devCount);
		vkEnumeratePhysicalDevices(m_instance, &devCount, physicalDevices.data());

		m_physicalDevice = physicalDevices[0];
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);
		
	}
}