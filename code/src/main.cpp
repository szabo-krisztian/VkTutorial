#include <iostream>

#include "instance.hpp"
#include "debug_messenger.hpp"

int main()
{
    glfwInit();

    try
    {
        tlr::Instance instance;
        tlr::DebugMessenger debugMessenger;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}