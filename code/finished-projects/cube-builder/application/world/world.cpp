#include "world.hpp"

#include <algorithm>

namespace tlr
{

static const std::vector<glm::vec3> DIRECTIONS =
{
    {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}, {0, 0, -1}
};

World::World()
{
    Initialize();
}

Block& World::GetBlock(const glm::ivec3& position)
{
    return _world[position.x + X_DIMENSION][position.y + Y_DIMENSION][position.z + Z_DIMENSION];
}

void World::Initialize()
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

    for (int x = -X_DIMENSION; x < X_DIMENSION; ++x)
    {
        for (int y = -Y_DIMENSION; y < Y_DIMENSION; ++y)
        {
            for (int z = -Z_DIMENSION; z < Z_DIMENSION; ++z)
            {
                glm::ivec3 blockPosition{x, y, z};
                GetBlock(blockPosition).Initialize(blockPosition);
            }
        }
    }
    GetBlock({0, 0, 0}).Place();
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
                Block block = GetBlock({x, y, z});
                if (block.IsPlaced())
                {
                    blocks.push_back(block);
                }
            }
        }
    }

    return blocks;
}

bool World::IsPositionInBounds(const glm::ivec3& position)
{
    auto absolutePosition = glm::abs(position);
    return absolutePosition.x <= X_DIMENSION && absolutePosition.y <= Y_DIMENSION && absolutePosition.z <= Z_DIMENSION;
}

void World::BuildBlock(const glm::vec3& playerPosition, const glm::vec3& ray)
{
    glm::vec3 rayEnd = playerPosition + glm::normalize(ray) * PLAYER_REACH_LENGTH;
    Block targetBlock = GetBlock(GetTargetBlockPosition(playerPosition, rayEnd));

    for (const auto& dir : DIRECTIONS)
    {
        glm::vec3 center = targetBlock.GetCenter();

        bool isFaceTowardsUs = glm::dot(dir, ray) < 0;
        if (!isFaceTowardsUs || GetBlock(static_cast<glm::ivec3>(static_cast<glm::vec3>(targetBlock.GetPosition()) + dir)).IsPlaced())
        {
            continue;
        }
        center += (dir / 2.0f);
        glm::vec3 offsetVector = (glm::vec3(1,1,1) - glm::abs(dir)) / 2.0f;
        

        glm::vec3 minFacePosition = center - offsetVector;
        glm::vec3 maxFacePosition = center + offsetVector;

        if (DoesRayIntersectCube(playerPosition, ray, minFacePosition, maxFacePosition))
        {
            GetBlock(GetPositionFromCenterPosition(center + dir)).Place();
            break;
        }
    }
    std::cout << "------------------" << std::endl;
}

glm::ivec3 World::GetPositionFromCenterPosition(const glm::vec3& centerPosition)
{
    return glm::ivec3(centerPosition.x - 0.5f, centerPosition.y - 0.5f, centerPosition.z - 0.5f);   
}

bool World::DoesRayIntersectCube(const glm::vec3& rayStart, const glm::vec3& rayDirection, const glm::vec3& cubeMin, const glm::vec3& cubeMax)
{
    glm::vec3 tMin = (cubeMin - rayStart) / rayDirection;
    glm::vec3 tMax = (cubeMax - rayStart) / rayDirection;

    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);

    float tEnter = std::max(std::max(t1.x, t1.y), t1.z);
    float tExit = std::min(std::min(t2.x, t2.y), t2.z);

    return tEnter <= tExit && tExit >= 0;
}

void World::BreakBlock(const glm::vec3& playerPosition, const glm::vec3& ray)
{
    glm::vec3 rayEnd = playerPosition + glm::normalize(ray) * PLAYER_REACH_LENGTH;
    glm::ivec3 targetBlockPosition = GetTargetBlockPosition(playerPosition, rayEnd);
    GetBlock(targetBlockPosition).Break();
}

glm::ivec3 World::GetTargetBlockPosition(const glm::vec3& rayStart, const glm::vec3& rayEnd)
{
    std::vector<glm::ivec3> blockPositions = GetIntersectedBlockPositions(rayStart, rayEnd);
    std::sort(blockPositions.begin(), blockPositions.end(), [&](const glm::ivec3& a, const glm::ivec3& b) {
        glm::vec3 vecA = GetBlock(a).GetCenter();
        glm::vec3 vecB = GetBlock(b).GetCenter();
        float distA = glm::length(rayStart - vecA);
        float distB = glm::length(rayStart - vecB);
        return distA < distB;
    });
    
    glm::ivec3 targetBlockPosition = blockPositions[1];

    for (std::size_t i = 1; i < blockPositions.size(); ++i)
    {
        assert(IsPositionInBounds(blockPositions[i]) && "you went out of the world!");

        if (GetBlock(blockPositions[i]).IsPlaced())
        {
            targetBlockPosition = blockPositions[i];
            break;
        }   
    }

    return targetBlockPosition;
}

bool Contains(const std::vector<glm::ivec3>& vec, const glm::ivec3& element)
{
    // Use std::find to search for the element
    return std::find(vec.begin(), vec.end(), element) != vec.end();
}

std::vector<glm::ivec3> World::GetIntersectedBlockPositions(const glm::vec3& rayStart, const glm::vec3& rayEnd)
{
    /*
     * Check out https://github.com/francisengelmann/fast_voxel_traversal
     */
    std::vector<glm::ivec3> visitedVoxels;

    glm::ivec3 currentVoxel(static_cast<int>(std::floor(rayStart.x)),
                            static_cast<int>(std::floor(rayStart.y)),
                            static_cast<int>(std::floor(rayStart.z)));

    glm::ivec3 lastVoxel(static_cast<int>(std::floor(rayEnd.x)),
                         static_cast<int>(std::floor(rayEnd.y)),
                         static_cast<int>(std::floor(rayEnd.z)));

    glm::vec3 ray = rayEnd - rayStart;

    int stepX = (ray.x >= 0) ? 1 : -1;
    int stepY = (ray.y >= 0) ? 1 : -1;
    int stepZ = (ray.z >= 0) ? 1 : -1;

    double nextVoxelBoundaryX = currentVoxel.x + (stepX > 0 ? 1 : 0);
    double nextVoxelBoundaryY = currentVoxel.y + (stepY > 0 ? 1 : 0);
    double nextVoxelBoundaryZ = currentVoxel.z + (stepZ > 0 ? 1 : 0);

    double tMaxX = (ray.x != 0) ? (nextVoxelBoundaryX - rayStart.x) / ray.x : DBL_MAX;
    double tMaxY = (ray.y != 0) ? (nextVoxelBoundaryY - rayStart.y) / ray.y : DBL_MAX;
    double tMaxZ = (ray.z != 0) ? (nextVoxelBoundaryZ - rayStart.z) / ray.z : DBL_MAX;

    double tDeltaX = (ray.x != 0) ? 1.0 / std::abs(ray.x) : DBL_MAX;
    double tDeltaY = (ray.y != 0) ? 1.0 / std::abs(ray.y) : DBL_MAX;
    double tDeltaZ = (ray.z != 0) ? 1.0 / std::abs(ray.z) : DBL_MAX;

    if (!Contains(visitedVoxels, currentVoxel))
    {
        visitedVoxels.push_back(currentVoxel);
    }
    

    while (currentVoxel != lastVoxel)
    {
        if (tMaxX < tMaxY)
        {
            if (tMaxX < tMaxZ)
            {
                currentVoxel.x += stepX;
                tMaxX += tDeltaX;
            }
            else
            {
                currentVoxel.z += stepZ;
                tMaxZ += tDeltaZ;
            }
        }
        else
        {
            if (tMaxY < tMaxZ)
            {
                currentVoxel.y += stepY;
                tMaxY += tDeltaY;
            }
            else
            {
                currentVoxel.z += stepZ;
                tMaxZ += tDeltaZ;
            }
        }
        if (!Contains(visitedVoxels, currentVoxel))
        {
            visitedVoxels.push_back(currentVoxel);
        }
    }

    return visitedVoxels;
}

} // namespace tlr