#include "my_window.hpp"

namespace tlr
{

MyWindow::MyWindow(MyInstance& instance) : mInstance(instance), mWindow(nullptr)
{
    InitWindow();

    if (mWindow == nullptr)
    {
        throw std::runtime_error("failed to create GLFW window!");
    }

    if (glfwCreateWindowSurface(mInstance.Get(), mWindow, nullptr, &mSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

MyWindow::~MyWindow()
{
    vkDestroySurfaceKHR(mInstance.Get(), mSurface, nullptr);
    glfwDestroyWindow(mWindow);
}

void MyWindow::InitWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(M_WIDTH, M_HEIGHT, "Vulkan", nullptr, nullptr);
}

const VkSurfaceKHR& MyWindow::GetSurface() const
{
    return mSurface;
}

GLFWwindow* MyWindow::GetWindow()
{
    return mWindow;
}

bool MyWindow::IsWindowActive()
{
    return !glfwWindowShouldClose(mWindow);
}

} // namespace tlr