#include "window.hpp"

namespace tlr
{

Window::Window() : mWindow(nullptr)
{
    InitWindow();

    if (mWindow == nullptr)
    {
        throw std::runtime_error("failed to create GLFW window!");
    }

    if (glfwCreateWindowSurface(StateBoard::instance->Get(), mWindow, nullptr, &mSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

    StateBoard::window = this;
}

Window::~Window()
{
    vkDestroySurfaceKHR(StateBoard::instance->Get(), mSurface, nullptr);
    glfwDestroyWindow(mWindow);
    StateBoard::window = nullptr;
}

void Window::InitWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(M_WIDTH, M_HEIGHT, "Vulkan", nullptr, nullptr);
}

const VkSurfaceKHR& Window::GetSurface() const
{
    return mSurface;
}

GLFWwindow* Window::GetWindow() const
{
    return mWindow;
}

bool Window::IsWindowActive() const
{
    return !glfwWindowShouldClose(mWindow);
}

} // namespace tlr