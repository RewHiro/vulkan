#pragma once

#include "vulkan_app_base.hpp"

#include <algorithm>

#include "glm/glm.hpp"
#include "GLTFSDK/GLTF.h"

namespace app
{
	class ModelApp : public VulkanAppBase
	{
	public:
		ModelApp() : VulkanAppBase() {}

		virtual void prepare() override;

	};
}