#include "model_app.hpp"

#include "stream_reader.hpp"
#include "stb_image.h"

namespace app
{
	void ModelApp::prepare()
	{
		auto modelFilePath = std::filesystem::path("alicia-solid.vrm");
		if (modelFilePath.is_relative())
		{
			auto current = std::filesystem::current_path();
			current /= modelFilePath;
			current.swap(modelFilePath);
		}

		auto reader = std::make_unique<StreamReader>(modelFilePath.parent_path());
		auto glbStream = reader->GetInputStream(modelFilePath.filename().u8string());
		auto glbResourceReader = std::make_shared<Microsoft::glTF::GLBResourceReader>(std::move(reader), std::move(glbStream));
		auto document = Microsoft::glTF::Deserialize(glbResourceReader->GetJson());

		makeModelGeometry( document, glbResourceReader);
		makeModelMaterial(document, glbResourceReader);

		prepareUniformBuffers();
	}

	void ModelApp::makeModelGeometry(const Microsoft::glTF::Document& document, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader)
	{
		using namespace glm;
		using namespace Microsoft::glTF;
		for (auto&& mesh : document.meshes.Elements())
		{
			for (auto&& meshPrimitive : mesh.primitives)
			{

				auto& idPosition = meshPrimitive.GetAttributeAccessorId(ACCESSOR_POSITION);
				auto& accessorPosition = document.accessors.Get(idPosition);

				auto& idNormal = meshPrimitive.GetAttributeAccessorId(ACCESSOR_NORMAL);
				auto& accessorNormal = document.accessors.Get(idNormal);

				auto& idUV = meshPrimitive.GetAttributeAccessorId(ACCESSOR_TEXCOORD_0);
				auto& accessorUV = document.accessors.Get(idUV);

				auto& idIndex = meshPrimitive.indicesAccessorId;
				auto& accessorIndex = document.accessors.Get(idIndex);
				
				auto vertexPosition = reader->ReadBinaryData<float>(document, accessorPosition);
				auto vertexNormal = reader->ReadBinaryData<float>(document, accessorNormal);
				auto vertexUV = reader->ReadBinaryData<float>(document, accessorUV);

				auto vertexCount = accessorPosition.count;
				std::vector<Vertex> vertices;
				for (uint32_t i = 0; i < vertexCount; ++i)
				{
					std::array<uint32_t, 3> threeElementsIds = { 3 * i, 3 * i + 1, 3 * i + 2 };
					std::array<uint32_t, 2> twoElementsIds = { 2 * i, 2 * i + 1 };

					vertices.emplace_back
					(
						Vertex
						{
							vec3(vertexPosition[threeElementsIds[0]], vertexPosition[threeElementsIds[1]], vertexPosition[threeElementsIds[2]]),
							vec3(vertexNormal[threeElementsIds[0]], vertexNormal[threeElementsIds[1]], vertexNormal[threeElementsIds[2]]),
							vec2(vertexUV[twoElementsIds[0]], vertexUV[twoElementsIds[1]])
						}
					);
				}

				std::vector<uint32_t> indices = reader->ReadBinaryData<uint32_t>(document, accessorIndex);

				auto vertexBufferSize = sizeof(Vertex) * vertices.size();
				auto indexBufferSize = sizeof(uint32_t) * indices.size();

				ModelMesh modelMesh{};
				modelMesh.vertexBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertices.data());
				modelMesh.indexBuffer = createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indices.data());
				modelMesh.vertexCount = vertices.size();
				modelMesh.indexCount = indices.size();
				modelMesh.materialIndex = document.materials.GetIndex(meshPrimitive.materialId);

				m_model.meshes.emplace_back(std::move(modelMesh));
			}
		}
	}

	void ModelApp::makeModelMaterial(const Microsoft::glTF::Document& document, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader)
	{
		for (auto&& material : document.materials.Elements())
		{
			auto textureId = material.metallicRoughness.baseColorTexture.textureId;
			if (textureId.empty())
			{
				textureId = material.normalTexture.textureId;
			}

			auto& texture = document.textures.Get(textureId);
			auto& image = document.images.Get(texture.imageId);
			auto imageBufferView = document.bufferViews.Get(image.bufferViewId);
			auto imageData = reader->ReadBinaryData<char>(document, imageBufferView);

			Material material{};
			material.alphaMode = material.alphaMode;
			material.texture = createTextureFromMemory(imageData);
			m_model.materials.push_back(std::move(material));
		}
	}

	ModelApp::BufferObject ModelApp::createBuffer(uint32_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags flags, const void* initialData) const
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

		if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 && initialData != nullptr)
		{
			void* data = nullptr;
			vkMapMemory(m_device, bufferObject.deviceMemory, 0, VK_WHOLE_SIZE, 0, &data);
			memcpy(data, initialData, size);
			vkUnmapMemory(m_device, bufferObject.deviceMemory);
		}

		return bufferObject;

	}

	ModelApp::TextureObject ModelApp::createTextureFromMemory(const std::vector<char>& imageData) const
	{
		BufferObject stagingBuffer{};
		TextureObject textureObject{};

		int width = 0, height = 0, channels = 0;
		auto* const image = stbi_load_from_memory( reinterpret_cast<const uint8_t*>(imageData.data()), imageData.size(), &width, &height, &channels, 0);
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
			stagingBuffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, image);
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

	void ModelApp::setImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const
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
		case VK_IMAGE_LAYOUT_UNDEFINED:
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

	void ModelApp::prepareUniformBuffers()
	{
		m_uniformBuffers.resize(m_swapchainImageViews.size());
		for (auto& uniformBuffer : m_uniformBuffers)
		{
			VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			uniformBuffer = createBuffer(sizeof(UniformParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, flags);
		}
	}
}