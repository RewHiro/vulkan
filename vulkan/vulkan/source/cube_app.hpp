#pragma once

#include "vulkan_app_base.hpp"
#include "glm/glm.hpp"

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

		CubeApp() :VulkanAppBase() {}

		virtual void prepare();

	private:
		void makeCubeGeometry();
	};
}