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
    (*this)[{0, 0, 0}] = true;
}

std::vector<Block> World::GetActiveBlocks()
{
    std::vector<Block> blocks;

    for (int x = -X_DIMENSION; x < X_DIMENSION; ++x)
    {
        for (int y = -Y_DIMENSION; y < Y_DIMENSION; ++y)
        {
            for (int z = -Z_DIMENSION; z < Z_DIMENSION; ++z)
            {
                Block block = (*this)[{x, y, z}];
                if (block)
                {
                    blocks.push_back(block);
                }
            }
        }
    }

    return blocks;
}

void World::BuildBlock(const glm::vec3& playerPosition, const glm::vec3& ray)
{
    glm::vec3 rayEnd = playerPosition + glm::normalize(ray) * PLAYER_REACH_LENGTH;
    glm::ivec3 targetBlockPosition = GetTargetBlockPosition(playerPosition, rayEnd);

    // TODO : find face of intersection
}

void World::BreakBlock(const glm::vec3& playerPosition, const glm::vec3& ray)
{
    glm::vec3 rayEnd = playerPosition + glm::normalize(ray) * PLAYER_REACH_LENGTH;
    glm::ivec3 targetBlockPosition = GetTargetBlockPosition(playerPosition, rayEnd);
    (*this)[targetBlockPosition] = false;
}

glm::ivec3 World::GetTargetBlockPosition(const glm::vec3& rayStart, const glm::vec3& rayEnd)
{
    std::vector<glm::ivec3> blockPositions = GetIntersectedBlockPositions(rayStart, rayEnd);
    glm::ivec3 targetBlockPosition = blockPositions[1];

    for (const auto& position : blockPositions)
    {
        if ((*this)[position])
        {
            targetBlockPosition = position;
            break;
        }
    }

    return targetBlockPosition;
}

std::vector<glm::ivec3> World::GetIntersectedBlockPositions(const glm::vec3& rayStart, const glm::vec3& rayEnd)
{
    std::vector<glm::ivec3> visitedVoxels;
    glm::ivec3 currentVoxel(static_cast<int>(std::floor(rayStart[0])), static_cast<int>(std::floor(rayStart[1])), static_cast<int>(std::floor(rayStart[2])));
    glm::ivec3 lastVoxel(static_cast<int>(std::floor(rayEnd[0])), static_cast<int>(std::floor(rayEnd[1])), static_cast<int>(std::floor(rayEnd[2])));
    glm::vec3 ray = rayEnd - rayStart;

    int stepX = (ray[0] >= 0) ? 1 : -1;
    int stepY = (ray[1] >= 0) ? 1 : -1;
    int stepZ = (ray[2] >= 0) ? 1 : -1;

    double nextVoxelBoundaryX = (currentVoxel[0] + stepX);
    double nextVoxelBoundaryY = (currentVoxel[1] + stepY);
    double nextVoxelBoundaryZ = (currentVoxel[2] + stepZ);

    double tMaxX = (ray[0] != 0) ? (nextVoxelBoundaryX - rayStart[0]) / ray[0] : DBL_MAX;
    double tMaxY = (ray[1] != 0) ? (nextVoxelBoundaryY - rayStart[1]) / ray[1] : DBL_MAX;
    double tMaxZ = (ray[2] != 0) ? (nextVoxelBoundaryZ - rayStart[2]) / ray[2] : DBL_MAX;

    double tDeltaX = (ray[0] != 0) ? ray[0] * stepX : DBL_MAX;
    double tDeltaY = (ray[1] != 0) ? ray[1] * stepY : DBL_MAX;
    double tDeltaZ = (ray[2] != 0) ? ray[2] * stepZ : DBL_MAX;

    glm::ivec3 diff(0, 0, 0);
    bool negRay = false;
    if (currentVoxel[0] != lastVoxel[0] && ray[0] < 0) { diff[0]--; negRay = true; }
    if (currentVoxel[1] != lastVoxel[1] && ray[1] < 0) { diff[1]--; negRay = true; }
    if (currentVoxel[2] != lastVoxel[2] && ray[2] < 0) { diff[2]--; negRay = true; }
    visitedVoxels.push_back(currentVoxel);
    if (negRay) {
        currentVoxel += diff;
        visitedVoxels.push_back(currentVoxel);
    }

    while (lastVoxel != currentVoxel) {
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                currentVoxel[0] += stepX;
                tMaxX += tDeltaX;
            } else {
                currentVoxel[2] += stepZ;
                tMaxZ += tDeltaZ;
            }
        } else {
            if (tMaxY < tMaxZ) {
                currentVoxel[1] += stepY;
                tMaxY += tDeltaY;
            } else {
                currentVoxel[2] += stepZ;
                tMaxZ += tDeltaZ;
            }
        }
        visitedVoxels.push_back(currentVoxel);
    }
    return visitedVoxels;
}

} // namespace tlr