#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "block.hpp"

namespace tlr
{
    
class WorldSpace
{
public:
    static constexpr int X_DIMENSION = 10;
    static constexpr int Y_DIMENSION = 10;
    static constexpr int Z_DIMENSION = 10;
    static const glm::ivec3 NULL_POSITION;

    WorldSpace();
    Block& operator[](const glm::ivec3& position);
    bool IsPositionInBounds(const glm::ivec3& position) const;

private:
    std::vector<std::vector<std::vector<Block>>> _world;
};

} // namespace tlr