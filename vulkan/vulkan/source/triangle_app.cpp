#include "triangle_app.hpp"
#include <array>

namespace app
{
	void TriangleApp::prepare()
	{

		constexpr glm::vec3 red(1.0f, 0.0f, 0.0f);
		constexpr glm::vec3 green(1.0f, 0.0f, 0.0f);
		constexpr glm::vec3 blue(1.0f, 0.0f, 0.0f);

		std::array<Vertex, 3> vertices =
		{
			Vertex{ glm::vec3(-1.0f,0.0f,0.0f),red },
			Vertex{ glm::vec3(+1.0f,0.0f,0.0f),green },
			Vertex{ glm::vec3(0.0f,1.0f,0.0f),blue }
		};

		std::array<uint32_t, 3>indices =
		{
			0,1,2
		};

		m_vertexBuffer = createBuffer(sizeof(vertices.data()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		m_indexBuffer = createBuffer(sizeof(indices.data()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		{
			void* data = nullptr;
			vkMapMemory(m_device, m_vertexBuffer.deviceMemory, 0, VK_WHOLE_SIZE, 0, &data);
			memcpy(data, vertices.data(), sizeof(vertices));
			vkUnmapMemory(m_device, m_vertexBuffer.deviceMemory);
		}

		{
			void* data = nullptr;
			vkMapMemory(m_device, m_indexBuffer.deviceMemory, 0, VK_WHOLE_SIZE, 0, &data);
			memcpy(data, indices.data(), sizeof(indices));
			vkUnmapMemory(m_device, m_indexBuffer.deviceMemory);
		}

		m_indexCount = indices.size();
	}

	TriangleApp::BufferObject TriangleApp::createBuffer(uint32_t size, VkBufferUsageFlags bufferUsageFlags) const
	{
		BufferObject bufferObject{};

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = bufferUsageFlags;
		bufferCreateInfo.size = size;
		auto result = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &bufferObject.buffer);
		checkResult(result);

		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(m_device, bufferObject.buffer, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		auto flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		memoryAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(memoryRequirements.memoryTypeBits, flags);
		vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &bufferObject.deviceMemory);

		vkBindBufferMemory(m_device, bufferObject.buffer, bufferObject.deviceMemory, 0);

		return bufferObject;
	}
}

