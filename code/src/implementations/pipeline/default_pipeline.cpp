#include "default_pipeline.hpp"

namespace tlr
{

DefaultPipeline::DefaultPipeline()
{
    SetVertexShaderSpvPath("spvs/vert.spv");
    SetFragmentShaderSpvPath("spvs/frag.spv");

    auto bindingDescriptions = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    SetVertexInputState(0, nullptr, 0, nullptr);
    SetLayout();
    BuildGraphicsPipeline();
}

} // namespace tlr