#include "test.hpp"

void Test::createVertexBuffer(const uint32_t size, VkDevice device)
{
	VkBuffer buffer = 0ull;
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.size = size;
	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	auto flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	//memoryAllocateInfo.memoryTypeIndex = //メモリタイプインデックス取得

	VkDeviceMemory deviceMemory = 0ull;
	vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);

	vkBindBufferMemory(device, buffer, deviceMemory, 0);
}
