#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "world_space.hpp"

namespace tlr
{

class World
{
public:
    static constexpr float PLAYER_REACH_LENGTH = 5;

    World();

    std::vector<Block> GetActiveBlocks();
    void               BuildBlock(const glm::vec3& playerPosition, const glm::vec3& ray);
    void               BreakBlock(const glm::vec3& playerPosition, const glm::vec3& ray);

private:
    WorldSpace _worldSpace;

    void                    Initialize();
    bool                    DoesRayIntersectCube(const glm::vec3& rayStart, const glm::vec3& rayDirection, const glm::vec3& cubeMin, const glm::vec3& cubeMax);
    glm::ivec3              GetTargetBlockPosition(const glm::vec3& rayStart, const glm::vec3& rayEnd);
    std::vector<glm::ivec3> GetIntersectedBlockPositions(const glm::vec3& rayStart, const glm::vec3& rayEnd);
};

} // namespace tlr