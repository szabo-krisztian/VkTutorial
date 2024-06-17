#include <iostream>

#include "app.hpp"

int main()
{
    glfwInit();

    try
    {
        tlr::App app;
        app.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}