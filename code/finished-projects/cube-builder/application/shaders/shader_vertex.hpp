#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "initializers.hpp"

namespace tlr
{

struct VertexInfo
{
    glm::vec3 pos;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription = init::VertexInputBindingDescription(0, sizeof(VertexInfo), VK_VERTEX_INPUT_RATE_VERTEX);
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0] = init::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos));
        return attributeDescriptions;
    }
};

} // namespace tlr