#include "first_app.hpp"

#include <cstdlib>
#include <stdexcept>
#include <iostream>

int main()
{
    vlk::FirstApp app;

    try
    {
        app.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}