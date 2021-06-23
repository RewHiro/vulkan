#include "cube_app.hpp"

constexpr auto WINDOW_WIDTH = 640;
constexpr auto WINDOW_HEIGHT = 480;
constexpr auto* APP_TITLE = "Test";

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, 0);
	auto window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_TITLE, nullptr, nullptr);

	app::CubeApp vulkanAppBase;
	vulkanAppBase.initialize(window, APP_TITLE);

	while (glfwWindowShouldClose(window) == GLFW_FALSE)
	{
		glfwPollEvents();
		vulkanAppBase.render();
	}

	vulkanAppBase.terminate();
	glfwTerminate();

	return 0;
}