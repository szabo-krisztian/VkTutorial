#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "block.hpp"

namespace tlr
{

class World
{
public:
    static constexpr int X_DIMENSION = 10;
    static constexpr int Y_DIMENSION = 10;
    static constexpr int Z_DIMENSION = 10;
    static constexpr float PLAYER_REACH_LENGTH = 8;

    World();

    std::vector<Block> GetActiveBlocks();
    void               BuildBlock(const glm::vec3& playerPosition, const glm::vec3& ray);
    void               BreakBlock(const glm::vec3& playerPosition, const glm::vec3& ray);

private:
    std::vector<std::vector<std::vector<Block>>> _world;

    void                    Initialize();
    Block&                  GetBlock(const glm::ivec3& position);
    bool                    IsPositionInBounds(const glm::ivec3& position);
    glm::ivec3              GetPositionFromCenterPosition(const glm::vec3& centerPosition);
    bool                    DoesRayIntersectCube(const glm::vec3& rayStart, const glm::vec3& rayDirection, const glm::vec3& cubeMin, const glm::vec3& cubeMax);
    glm::ivec3              GetTargetBlockPosition(const glm::vec3& rayStart, const glm::vec3& rayEnd);
    std::vector<glm::ivec3> GetIntersectedBlockPositions(const glm::vec3& rayStart, const glm::vec3& rayEnd);
};

} // namespace tlr