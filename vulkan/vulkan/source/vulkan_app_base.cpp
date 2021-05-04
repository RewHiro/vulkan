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

		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = appName;
		applicationInfo.pEngineName = appName;
		applicationInfo.applicationVersion = VK_API_VERSION_1_1;
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

		const char* layers[] = { "VK_LAYTER_LUNARG_standard_validation" };
		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.enabledLayerCount = 1;
		instanceCreateInfo.ppEnabledLayerNames = layers;
		instanceCreateInfo.enabledExtensionCount = extensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		auto result =  vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);

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

	void VulkanAppBase::searchGraphicsQueueIndex()
	{
		uint32_t propertyCount;
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

	}
}