#include "vlk_window.hpp"

namespace vlk
{

VlkWindow::VlkWindow(int width, int height, std::string windowName) : mWidth(width), mHeight(height), mWindowName(windowName)
{
    InitWindow();
}

VlkWindow::~VlkWindow()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

bool VlkWindow::ShouldClose()

{
    return glfwWindowShouldClose(mWindow);
}

void VlkWindow::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
}

} // namespace vlk