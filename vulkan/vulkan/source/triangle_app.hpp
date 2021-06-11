#pragma once
#include "vulkan_app_base.hpp"
#include "glm/glm.hpp"

namespace app
{
	class TriangleApp : public VulkanAppBase
	{
	public:

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
		};

		struct BufferObject
		{
			VkBuffer buffer;
			VkDeviceMemory deviceMemory;
		};

		TriangleApp() :
			VulkanAppBase(){}

		virtual void prepare() override;

	private:

		BufferObject createBuffer(uint32_t size, VkBufferUsageFlags bufferUsageFlags) const;

		BufferObject m_vertexBuffer{};
		BufferObject m_indexBuffer{};

		uint32_t m_indexCount = 0;

	};
}