#pragma once

#include "vlk_window.hpp"

namespace vlk
{

class FirstApp
{
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    
    void Run();
private:
    VlkWindow mVlkWindow{WIDTH, HEIGHT, "Hello, Vulkan!"};
};

} // namespace vlk