#pragma once

#include "vulkan_app_base.hpp"

#include <algorithm>

#include "glm/glm.hpp"
#include "GLTFSDK/GLTF.h"

namespace Microsoft::glTF
{
	class Document;
	class GLTFResourceReader;
}

namespace app
{
	class ModelApp : public VulkanAppBase
	{
	public:

		struct Vertex
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

		struct TextureObject
		{
			VkImage image;
			VkDeviceMemory deviceMemory;
			VkImageView imageView;
		};

		struct ModelMesh
		{
			BufferObject vertexBuffer;
			BufferObject indexBuffer;
			uint32_t vertexCount;
			uint32_t indexCount;
			int materialIndex;
			std::vector<VkDescriptorSet> descriptorSets;
		};

		struct Material
		{
			TextureObject texture;
			Microsoft::glTF::AlphaMode alphaMode;
		};

		struct Model
		{
			std::vector<ModelMesh> meshes;
			std::vector<Material> materials;
		};

		ModelApp() : VulkanAppBase() {}

		virtual void prepare() override;

	private:
		void makeModelGeometry(const Microsoft::glTF::Document&, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader);

		BufferObject createBuffer(uint32_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags flags, const void* initialData) const;


		Model m_model{};
	};
}