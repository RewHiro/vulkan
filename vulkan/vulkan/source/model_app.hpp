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

		struct UniformParameters
		{
			glm::mat4 matrixWorld;
			glm::mat4 matrixView;
			glm::mat4 matrixProjection;
		};

		ModelApp() : VulkanAppBase() {}

		virtual void prepare() override;
		virtual void makeCommand(VkCommandBuffer command) override;

	private:
		void makeModelGeometry(const Microsoft::glTF::Document&, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader);
		void makeModelMaterial(const Microsoft::glTF::Document&, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader);

		BufferObject createBuffer(uint32_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags flags, const void* initialData = nullptr) const;
		TextureObject createTextureFromMemory(const std::vector<char>& imageData)const;
		VkSampler createSampler()const;

		void setImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const;

		VkPipelineShaderStageCreateInfo loadShaderModule(std::string_view fileName, VkShaderStageFlagBits stage);

		void prepareUniformBuffers();
		void prepareDescriptorSetLayout();
		void prepareDescriptorPool();
		void prepareDescriptorSet();


		Model m_model{};

		std::vector<BufferObject> m_uniformBuffers;

		VkDescriptorSetLayout m_descriptorSetLayout = 0ull;
		VkDescriptorPool m_descriptorPool = 0ull;

		VkSampler m_sampler = 0ull;

		std::vector<VkDescriptorSet> m_descriptorSet;
		TextureObject m_textureObject{};

		VkPipelineLayout m_pipelineLayout = 0ull;
		VkPipeline m_pipelineOpaque = 0ull;
		VkPipeline m_pipelineAlpha = 0ull;
	};
}