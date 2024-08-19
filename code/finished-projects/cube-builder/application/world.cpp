#include "world.hpp"

namespace tlr
{

World::World()
{
    Initialize();
}

Block& World::operator[](glm::ivec3 position)
{
    return _world[position.x + X_DIMENSION][position.y + Y_DIMENSION][position.z + Z_DIMENSION];
}

void World::Initialize()
{
    for (int x = -X_DIMENSION; x < X_DIMENSION; ++x)
    {
        for (int y = -Y_DIMENSION; y < Y_DIMENSION; ++y)
        {
            for (int z = -Z_DIMENSION; z < Z_DIMENSION; ++z)
            {
                (*this)[{x, y, z}].Initialize({x, y, z});
            }
        }
    }
}

} // namespace tlr