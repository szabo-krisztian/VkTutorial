#include "default_pipeline.hpp"

namespace tlr
{

DefaultPipeline::DefaultPipeline()
{
    SetVertexShaderSpvPath("spvs/vert.spv");
    SetFragmentShaderSpvPath("spvs/frag.spv");

    auto bindingDescriptions = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    SetVertexInputState(1, &bindingDescriptions, 2, attributeDescriptions.data());
    // descriptor
    BuildGraphicsPipeline();
}

} // namespace tlr