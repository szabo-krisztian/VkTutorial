#include "world_space.hpp"

namespace tlr
{

const glm::ivec3 WorldSpace::NULL_POSITION = glm::ivec3{WorldSpace::X_DIMENSION + 1, WorldSpace::Y_DIMENSION + 1, WorldSpace::Z_DIMENSION + 1};

WorldSpace::WorldSpace()
{
    std::size_t xSize = X_DIMENSION * 2 + 1;
    std::size_t ySize = Y_DIMENSION * 2 + 1;
    std::size_t zSize = Z_DIMENSION * 2 + 1;

    _world.resize(xSize);
    for (int i = 0; i < xSize; ++i)
    {
        _world[i].resize(ySize);
        for (int j = 0; j < ySize; ++j)
        {
            _world[i][j].resize(zSize);
        }
    }
}

Block WorldSpace::operator[](const glm::ivec3& position) const
{
    return _world[position.x + X_DIMENSION][position.y + Y_DIMENSION][position.z + Z_DIMENSION];
}

Block& WorldSpace::operator[](const glm::ivec3& position)
{
    return _world[position.x + X_DIMENSION][position.y + Y_DIMENSION][position.z + Z_DIMENSION];
}

bool WorldSpace::IsPositionInBounds(const glm::ivec3& position) const
{
    auto absolutePosition = glm::abs(position);
    return absolutePosition.x <= X_DIMENSION && absolutePosition.y <= Y_DIMENSION && absolutePosition.z <= Z_DIMENSION;
}

} // namespace tlr