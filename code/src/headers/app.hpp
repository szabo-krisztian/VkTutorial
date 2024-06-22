#pragma once

#include "state_board.hpp"
#include "default_pipeline.hpp"

namespace tlr
{

class App
{
public:
    App();
    

private:
    DefaultPipeline pipeline;

    std::vector<VkFramebuffer> mFramebuffers;
    void InitFramebuffers();
};

} // namespace tlr