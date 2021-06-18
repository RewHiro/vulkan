#include "cube_app.hpp"

#include <array>

namespace app
{
	void CubeApp::prepare()
	{

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
				//ê≥ñ 
				{ vec3(-k,k,k),yellow,leftBottom },
				{ vec3(-k,-k,k),red,leftTop},
				{ vec3(k,k,k),white,rightBottom },
				{ vec3(k,-k,k),magenta,rightTop },

				//âE
				{ vec3(k,k,k),white,leftBottom },
				{ vec3(k,-k,k),magenta,leftTop},
				{ vec3(k,k,-k),cyan,rightBottom },
				{ vec3(k,-k,-k),blue,rightTop },

				//ç∂
				{ vec3(-k,k,-k),green,leftBottom },
				{ vec3(-k,-k,-k),black,leftTop},
				{ vec3(-k,k,k),yellow,rightBottom },
				{ vec3(-k,-k,k),red,rightTop },

				//ó†
				{ vec3(k,k,-k),cyan,leftBottom },
				{ vec3(k,-k,-k),blue,leftTop},
				{ vec3(-k,k,-k),green,rightBottom },
				{ vec3(-k,-k,-k),black,rightTop },

				//è„
				{ vec3(-k,k,-k),green,leftBottom },
				{ vec3(-k,k,k),yellow,leftTop},
				{ vec3(k,k,-k),cyan,rightBottom },
				{ vec3(k,k,k),white,rightTop },

				//â∫
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

	}

}

