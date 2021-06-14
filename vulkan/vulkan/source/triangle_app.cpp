#include "triangle_app.hpp"
#include <array>
#include <fstream>

namespace app
{
	void TriangleApp::prepare()
	{

		constexpr glm::vec3 red(1.0f, 0.0f, 0.0f);
		constexpr glm::vec3 green(0.0f, 1.0f, 0.0f);
		constexpr glm::vec3 blue(0.0f, 0.0f, 1.0f);

		std::array<Vertex, 3> vertices =
		{
			Vertex{ glm::vec3(-1.0f,0.0f,0.0f),red },
			Vertex{ glm::vec3(+1.0f,0.0f,0.0f),blue },
			Vertex{ glm::vec3(0.0f,1.0f,0.0f),green }
		};

		std::array<uint32_t, 3>indices =
		{
			0,1,2
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

		VkVertexInputBindingDescription vertexInputBindingDescription
		{
			0,
			sizeof(Vertex),
			VK_VERTEX_INPUT_RATE_VERTEX
		};

		std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributeDescription
		{
			VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex,position)},
			VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex,color)}
		};

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
		pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescription.size();
		pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription.data();

		const auto colorWriteAll =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
		pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
		pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		pipelineColorBlendAttachmentState.colorWriteMask = colorWriteAll;
		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
		pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineColorBlendStateCreateInfo.attachmentCount = 1;
		pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

		VkViewport viewport
		{
			0.0f, static_cast<float>(m_swapchainExtent.height),
			static_cast<float>(m_swapchainExtent.width), -1.0f * m_swapchainExtent.height,
			0.0f,1.0f
		};

		VkRect2D scissor =
		{
			{0,0},
			m_swapchainExtent
		};
		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
		pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportStateCreateInfo.viewportCount = 1;
		pipelineViewportStateCreateInfo.pViewports = &viewport;
		pipelineViewportStateCreateInfo.scissorCount = 1;
		pipelineViewportStateCreateInfo.pScissors = &scissor;

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
		pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
		pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
		pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
		pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
		pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

		std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos
		{
			loadShaderModule("source/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			loadShaderModule("source/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		};

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.stageCount = pipelineShaderStageCreateInfos.size();
		graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();
		graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
		graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
		graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
		graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
		graphicsPipelineCreateInfo.renderPass = m_renderPass;
		graphicsPipelineCreateInfo.layout = m_pipelineLayout;
		vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_pipeline);

		for (const auto& shaderStage : pipelineShaderStageCreateInfos)
		{
			vkDestroyShaderModule(m_device, shaderStage.module, nullptr);
		}
	}

	void TriangleApp::cleanup()
	{
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		vkDestroyPipeline(m_device, m_pipeline, nullptr);

		vkFreeMemory(m_device, m_vertexBuffer.deviceMemory, nullptr);
		vkFreeMemory(m_device, m_indexBuffer.deviceMemory, nullptr);
		vkDestroyBuffer(m_device, m_vertexBuffer.buffer, nullptr);
		vkDestroyBuffer(m_device, m_indexBuffer.buffer, nullptr);
	}

	void TriangleApp::makeCommand(VkCommandBuffer command)
	{
		vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command, 0, 1, &m_vertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(command, m_indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command, m_indexCount, 1, 0, 0, 0);
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

	VkPipelineShaderStageCreateInfo TriangleApp::loadShaderModule(const std::string& fileName, VkShaderStageFlagBits stage)
	{
		std::ifstream infile( fileName, std::ios::binary);

		if (!infile)
		{
			OutputDebugStringA(fileName.data());
			OutputDebugStringA("file not found.\n");
			DebugBreak();
		}

		std::vector<char>filedata;
		filedata.resize(static_cast<uint32_t>(infile.seekg(0, std::ifstream::end).tellg()));
		infile.seekg(0, std::ifstream::beg).read(filedata.data(), filedata.size());

		VkShaderModule shaderModule = 0ull;
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(filedata.data());
		shaderModuleCreateInfo.codeSize = filedata.size();
		vkCreateShaderModule(m_device, &shaderModuleCreateInfo, nullptr, &shaderModule);

		VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{};
		pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineShaderStageCreateInfo.stage = stage;
		pipelineShaderStageCreateInfo.module = shaderModule;
		pipelineShaderStageCreateInfo.pName = "main";

		return pipelineShaderStageCreateInfo;
	}
}

