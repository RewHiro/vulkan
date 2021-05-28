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
		initializeInstance(appName);

		selectPhysicalDevice();
		m_graphicsQueueIndex = searchGraphicsQueueIndex();

		createDevice();

		prepareCommandPool();
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

	void VulkanAppBase::checkResult( VkResult result )
	{
		if (result == VK_SUCCESS) return;
		DebugBreak();
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
		checkResult(result);

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
			vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &count, nullptr);
			deviceExtensionsPropeties.resize(count);
			vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &count, deviceExtensionsPropeties.data());
		}


		std::vector<const char*> extensions;
		for (const auto& deviceExtensionsProperty : deviceExtensionsPropeties)
		{
			extensions.emplace_back(deviceExtensionsProperty.extensionName);
		}

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
		deviceCreateInfo.enabledExtensionCount = extensions.size();
		deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
		deviceCreateInfo.queueCreateInfoCount = 1;

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
}