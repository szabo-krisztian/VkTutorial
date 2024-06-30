#include "app_base.hpp"

int main()
{
    glfwInit();
    
    try
    {
        tlr::AppBase appBase;
        appBase.Init();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}