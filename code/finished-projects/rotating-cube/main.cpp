#include <iostream>

#include "app.hpp"

int main()
{
    try
    {
        tlr::App app;        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}