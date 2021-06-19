#include "cube_app.hpp"

#include <array>

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

}

