#include "model_app.hpp"

#include "stream_reader.hpp"

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
}