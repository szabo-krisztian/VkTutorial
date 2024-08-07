#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "initializers.hpp"

namespace tlr
{

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription = init::VertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0] = init::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
        attributeDescriptions[1] = init::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
        return attributeDescriptions;
    }
};

} // namespace tlr