#pragma once

#include "vlk_window.hpp"
#include "vlk_pipeline.hpp"

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
    VlkPipeline mVlkPipeline{"../src/shaders/simple_shader.vert.spv", "../src/shaders/simple_shader.frag.spv"};
};

} // namespace vlk