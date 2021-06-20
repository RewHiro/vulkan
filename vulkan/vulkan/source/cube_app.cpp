#include "cube_app.hpp"

#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace app
{
	void CubeApp::prepare()
	{
		makeCubeGeometry();
		prepareUniformBuffers();
		prepareDescriptorSetLayout();
		prepareDescriptorPool();
	}

	void CubeApp::makeCubeGeometry()
	{
		using namespace glm;

		constexpr auto k = 1.0f;

		constexpr vec3 red(1.0f, 0.0f, 0.0f);
		constexpr vec3 green(0.0f, 1.0f, 0.0f);
		constexpr vec3 blue(0.0f, 0.0f, 1.0f);

		constexpr vec3 white(1.0f);
		constexpr vec3 black(0.0f);

		constexpr vec3 yellow(1.0f, 1.0f, 0.0f);
		constexpr vec3 magenta(1.0f, 0.0f, 1.0f);
		constexpr vec3 cyan(0.0f, 1.0f, 1.0f);

		constexpr vec2 leftBottom(0.0f, 0.0f);
		constexpr vec2 leftTop(0.0f, 1.0f);
		constexpr vec2 rightBottom(1.0f, 0.0f);
		constexpr vec2 rightTop(1.0f, 1.0f);

		std::array<CubeVertex, 24> vertices =
		{ {
				//正面
				{ vec3(-k,k,k),yellow,leftBottom },
				{ vec3(-k,-k,k),red,leftTop},
				{ vec3(k,k,k),white,rightBottom },
				{ vec3(k,-k,k),magenta,rightTop },

				//右
				{ vec3(k,k,k),white,leftBottom },
				{ vec3(k,-k,k),magenta,leftTop},
				{ vec3(k,k,-k),cyan,rightBottom },
				{ vec3(k,-k,-k),blue,rightTop },

				//左
				{ vec3(-k,k,-k),green,leftBottom },
				{ vec3(-k,-k,-k),black,leftTop},
				{ vec3(-k,k,k),yellow,rightBottom },
				{ vec3(-k,-k,k),red,rightTop },

				//裏
				{ vec3(k,k,-k),cyan,leftBottom },
				{ vec3(k,-k,-k),blue,leftTop},
				{ vec3(-k,k,-k),green,rightBottom },
				{ vec3(-k,-k,-k),black,rightTop },

				//上
				{ vec3(-k,k,-k),green,leftBottom },
				{ vec3(-k,k,k),yellow,leftTop},
				{ vec3(k,k,-k),cyan,rightBottom },
				{ vec3(k,k,k),white,rightTop },

				//下
				{ vec3(-k,-k,k),red,leftBottom },
				{ vec3(-k,-k,-k),black,leftTop},
				{ vec3(k,-k,k),magenta,rightBottom },
				{ vec3(k,-k,-k),blue,rightTop }
			} };

		std::array<uint32_t, 36> indices =
		{
			0,2,1, 1,2,3, //front
			4,6,5, 5,6,7, //right
			8,10,9, 9,10,11, //left

			12,14,13, 13,14,15, //back
			16,18,17, 17,18,19, //top
			20,22,21, 21,22,23 //bottom
		};

		m_vertexBuffer = createBuffer(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		m_indexBuffer = createBuffer(sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

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

	void CubeApp::prepareUniformBuffers()
	{
		m_uniformBuffers.resize(m_swapchainImageViews.size());
		for (auto& uniformBuffer : m_uniformBuffers)
		{
			VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			uniformBuffer = createBuffer(sizeof(UniformParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, flags);
		}
	}

	void CubeApp::prepareDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding>bindings;
		VkDescriptorSetLayoutBinding bindingUniformBuffer{}, bindingTexture{};
		bindingUniformBuffer.binding = 0;
		bindingUniformBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindingUniformBuffer.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindingUniformBuffer.descriptorCount = 1;
		bindings.emplace_back(std::move(bindingUniformBuffer));

		bindingTexture.binding = 1;
		bindingTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindingTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindingTexture.descriptorCount = 1;
		bindings.emplace_back(std::move(bindingTexture));

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindings.size();
		createInfo.pBindings = bindings.data();
		vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &m_descriptorSetLayout);
	}

	void CubeApp::prepareDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> descriptorPoolSize;
		descriptorPoolSize[0].descriptorCount = m_swapchainImageViews.size();
		descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSize[1].descriptorCount = m_swapchainImageViews.size();
		descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.maxSets = m_swapchainImageViews.size();
		createInfo.poolSizeCount = descriptorPoolSize.size();
		createInfo.pPoolSizes = descriptorPoolSize.data();
		vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_descriptorPool);
	}

	void CubeApp::prepareDescriptorSet()
	{
		std::vector<VkDescriptorSetLayout>descriptorSetLayouts;
		for (auto i = 0u; i < m_swapchainImageViews.size(); i++)
		{
			descriptorSetLayouts.push_back(m_descriptorSetLayout);
		}

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_swapchainImageViews.size();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		m_descriptorSet.resize(m_swapchainImageViews.size());
		vkAllocateDescriptorSets(m_device, &descriptorSetAllocateInfo, m_descriptorSet.data());

		for (auto i = 0u; i < m_swapchainImageViews.size(); i++)
		{
			VkDescriptorBufferInfo descriptorBufferInfo{};
			descriptorBufferInfo.buffer = m_uniformBuffers[i].buffer;

			//TODO:m_uniformBuffers分のサイズで十分か試す
			descriptorBufferInfo.range = VK_WHOLE_SIZE;

			VkDescriptorImageInfo descriptorImageInfo{};
			descriptorImageInfo.imageView = m_textureObject.imageView;
			descriptorImageInfo.sampler = m_sampler;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet uniformBufferWriteDescriptorSet{};
			uniformBufferWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uniformBufferWriteDescriptorSet.dstBinding = 0;
			uniformBufferWriteDescriptorSet.descriptorCount = 1;
			uniformBufferWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformBufferWriteDescriptorSet.pBufferInfo = &descriptorBufferInfo;
			uniformBufferWriteDescriptorSet.dstSet = m_descriptorSet[i];

			VkWriteDescriptorSet textureWriteDescriptorSet{};
			textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			textureWriteDescriptorSet.dstBinding = 1;
			textureWriteDescriptorSet.descriptorCount = 1;
			textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureWriteDescriptorSet.pImageInfo = &descriptorImageInfo;
			textureWriteDescriptorSet.dstSet = m_descriptorSet[i];

			std::vector<VkWriteDescriptorSet> writeDescriptorSets =
			{
				uniformBufferWriteDescriptorSet,
				textureWriteDescriptorSet,
			};
			vkUpdateDescriptorSets(m_device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	CubeApp::BufferObject CubeApp::createBuffer(uint32_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags flags) const
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
		memoryAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(memoryRequirements.memoryTypeBits, flags);
		vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &bufferObject.deviceMemory);

		vkBindBufferMemory(m_device, bufferObject.buffer, bufferObject.deviceMemory, 0);

		return bufferObject;
	}

	CubeApp::TextureObject CubeApp::createTextureObject(const std::string& fileName) const
	{
		BufferObject stagingBuffer{};
		TextureObject textureObject{};

		int width = 0, height = 0, channels = 0;
		auto* const image = stbi_load(fileName.data(), &width, &height, &channels, 0);
		auto format = VK_FORMAT_R8G8B8A8_UNORM;

		{
			VkImageCreateInfo imageCreateInfo{};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
			imageCreateInfo.format = format;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			vkCreateImage(m_device, &imageCreateInfo, nullptr, &textureObject.image);

			VkMemoryRequirements memoryRequirements{};
			vkGetImageMemoryRequirements(m_device, textureObject.image, &memoryRequirements);
			VkMemoryAllocateInfo memoryAllocateInfo{};
			memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocateInfo.allocationSize = memoryRequirements.size;
			memoryAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &textureObject.deviceMemory);
			vkBindImageMemory(m_device, textureObject.image, textureObject.deviceMemory, 0);
		}

		{
			uint32_t imageSize = width * height * sizeof(uint32_t);
			stagingBuffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			void* data = nullptr;
			vkMapMemory(m_device, stagingBuffer.deviceMemory, 0, VK_WHOLE_SIZE, 0, &data);
			memcpy(data, image, imageSize);
			vkUnmapMemory(m_device, stagingBuffer.deviceMemory);
		}

		VkBufferImageCopy copyRegion{};
		copyRegion.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		copyRegion.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT,0,0,1 };
		VkCommandBuffer commandBuffer = nullptr;
		{
			VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
			commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferAllocateInfo.commandBufferCount = 1;
			commandBufferAllocateInfo.commandPool = m_commandPool;
			commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &commandBuffer);
		}

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		setImageMemoryBarrier(commandBuffer, textureObject.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, textureObject.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		setImageMemoryBarrier(commandBuffer, textureObject.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(m_deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
		{
			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.image = textureObject.image;
			imageViewCreateInfo.format = format;
			imageViewCreateInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A,
			};
			imageViewCreateInfo.subresourceRange = {
				VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1
			};
			vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &textureObject.imageView);
		}

		vkDeviceWaitIdle(m_device);
		vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);

		vkFreeMemory(m_device, stagingBuffer.deviceMemory, nullptr);
		vkDestroyBuffer(m_device, stagingBuffer.buffer, nullptr);

		stbi_image_free(image);

		return textureObject;
	}

	void CubeApp::setImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };
		imageMemoryBarrier.image = image;

		VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		switch (oldLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED :
			imageMemoryBarrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}

		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		}

		vkCmdPipelineBarrier
		(
			commandBuffer,
			srcStageFlags,
			dstStageFlags,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imageMemoryBarrier
		);
	}

}

