#include "first_app.hpp"

namespace vlk
{

void FirstApp::Run()
{
    while (!mVlkWindow.ShouldClose())
    {
        glfwPollEvents();
    }
}

} //namespace vlk