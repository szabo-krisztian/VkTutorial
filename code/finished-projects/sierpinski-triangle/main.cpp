#include "app.hpp"

int main()
{
    try
    {
        int fractalDepth = 5;
        tlr::App app(fractalDepth);
        app.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
