#include <iostream>

#include "app.hpp"

int main()
{
    try
    {
        tlr::App app;
        app.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}