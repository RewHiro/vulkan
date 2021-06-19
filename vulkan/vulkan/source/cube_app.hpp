#pragma once

#include "vulkan_app_base.hpp"
#include "glm/glm.hpp"

#include <vector>

namespace app
{
	class CubeApp : public VulkanAppBase
	{
	public:

		struct CubeVertex
		{
			glm::vec3 position;
			glm::vec3 color;
			glm::vec2 uv;
		};

		struct BufferObject
		{
			VkBuffer buffer;
			VkDeviceMemory deviceMemory;
		};

		struct UniformParameters
		{
			glm::mat4 matrixWorld;
			glm::mat4 matrixView;
			glm::mat4 matrixProjection;
		};

		CubeApp() :VulkanAppBase() {}

		virtual void prepare();

	private:
		void makeCubeGeometry();
		void prepareUniformBuffers();

		BufferObject createBuffer(uint32_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) const;

		BufferObject m_vertexBuffer{};
		BufferObject m_indexBuffer{};
		uint32_t m_indexCount = 0;

		std::vector<BufferObject>m_uniformBuffers;
	};
}