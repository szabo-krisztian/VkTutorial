#pragma once

#include "state_board.hpp"
#include "pipeline_core.hpp"

namespace tlr
{

class App
{
public:
    App();
    

private:
    VkPipeline mPipeline;

    std::vector<VkFramebuffer> mFramebuffers;

    void InitFramebuffers();
};

} // namespace tlr