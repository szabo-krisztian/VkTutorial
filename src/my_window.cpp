#include "my_window.hpp"

namespace tlr
{

MyVulkanWindow::MyVulkanWindow(MyVulkanInstance& instance) : mInstance(instance), mWindow(nullptr)
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


MyVulkanWindow::~MyVulkanWindow()
{
    vkDestroySurfaceKHR(mInstance.Get(), mSurface, nullptr);
    glfwDestroyWindow(mWindow);
}

void MyVulkanWindow::InitWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(M_WIDTH, M_HEIGHT, "Vulkan", nullptr, nullptr);
}

const VkSurfaceKHR& MyVulkanWindow::GetSurface() const
{
    return mSurface;
}

const GLFWwindow* MyVulkanWindow::GetWindow() const
{
    return mWindow;
}

bool MyVulkanWindow::IsWindowActive()
{
    return !glfwWindowShouldClose(mWindow);
}

} // namespace tlr