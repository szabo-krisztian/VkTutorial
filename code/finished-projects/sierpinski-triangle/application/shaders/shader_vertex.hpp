#pragma once

#include "vulkan/vulkan.h"
#include "initializers.hpp"
#include "glm/glm.hpp"
#include <array>
#include <vector>

namespace tlr
{

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription = init::VertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0] = init::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos));
        attributeDescriptions[1] = init::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
        return attributeDescriptions;
    }
};

} // namespace tlr