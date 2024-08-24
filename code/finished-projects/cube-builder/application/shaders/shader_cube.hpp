#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "initializers.hpp"

namespace tlr
{

// TODO: passing mat4 to shader

struct CubeInfo
{
    glm::vec3 color;
    glm::mat4 transform;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription = init::VertexInputBindingDescription(1, sizeof(CubeInfo), VK_VERTEX_INPUT_RATE_INSTANCE);
        return bindingDescription;
    }

};

} // namespace tlr