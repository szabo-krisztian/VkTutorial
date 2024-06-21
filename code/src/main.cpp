#include <iostream>

#include "instance.hpp"
#include "debug_messenger.hpp"
#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"

int main()
{
    glfwInit();

    try
    {
        tlr::Instance instance;
        tlr::DebugMessenger debugMessenger;
        tlr::Window window;
        tlr::Device device;
        tlr::Swapchain swapchain;

        while (window.IsWindowActive())
        {
            glfwPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}