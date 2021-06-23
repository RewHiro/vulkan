#pragma once

#include "vulkan_app_base.hpp"
#include "glm/glm.hpp"

#include <vector>
#include <string>
#include <string_view>

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

		struct TextureObject
		{
			VkImage image;
			VkDeviceMemory deviceMemory;
			VkImageView imageView;
		};

		struct ShaderParameters
		{
			glm::mat4 matrixWorld;
			glm::mat4 matrixView;
			glm::mat4 matrixProjection;
		};

		CubeApp() :VulkanAppBase() {}

		virtual void prepare() override;
		virtual void cleanup() override;
		virtual void makeCommand(VkCommandBuffer command) override;

	private:
		void makeCubeGeometry();
		void prepareUniformBuffers();
		void prepareDescriptorSetLayout();
		void prepareDescriptorPool();
		void prepareDescriptorSet();

		BufferObject createBuffer(uint32_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) const;

		TextureObject createTextureObject(std::string_view fileName) const;

		VkSampler createSampler()const;

		void setImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const;

		VkPipelineShaderStageCreateInfo loadShaderModule(std::string_view fileName, VkShaderStageFlagBits stage);

		BufferObject m_vertexBuffer{};
		BufferObject m_indexBuffer{};
		uint32_t m_indexCount = 0;

		std::vector<BufferObject>m_uniformBuffers;

		VkDescriptorSetLayout m_descriptorSetLayout = 0ull;
		VkDescriptorPool m_descriptorPool = 0ull;
		std::vector<VkDescriptorSet> m_descriptorSet;

		TextureObject m_textureObject{};
		VkSampler m_sampler = 0ull;

		VkPipelineLayout m_pipelineLayout = 0ull;
		VkPipeline m_pipeline = 0ull;
	};
}