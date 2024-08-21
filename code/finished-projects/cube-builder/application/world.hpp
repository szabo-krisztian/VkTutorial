#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <vector>

namespace tlr
{

class Block
{
public:
    void Initialize(const glm::ivec3& position)
    {
        assert(!_isInitialized && "position cannot be changed after initialization!");
        _position = position;
    }

    glm::ivec3 GetPosition() const
    {
        return _position;
    }

    glm::vec3 GetCenter() const
    {
        return static_cast<glm::vec3>(_position) + glm::vec3(0.5f, 0.5f, 0.5f);
    }

    void operator=(bool value)
    {
        _isActive = value;
    }

    operator bool() const
    {
        return _isActive;
    }

private:
    glm::ivec3 _position;
    bool _isActive = false;
    bool _isInitialized = false;
};

class World
{
public:
    static constexpr int X_DIMENSION = 10;
    static constexpr int Y_DIMENSION = 10;
    static constexpr int Z_DIMENSION = 10;
    static constexpr float PLAYER_REACH_LENGTH = 3;

    World();

    std::vector<Block> GetActiveBlocks();
    Block&             operator[](glm::ivec3 position);
    void               BuildBlock(const glm::vec3& playerPosition, const glm::vec3& ray);
    void               BreakBlock(const glm::vec3& playerPosition, const glm::vec3& ray);

private:
    Block _world[X_DIMENSION * 2 + 1][X_DIMENSION * 2 + 1][X_DIMENSION * 2 + 1];

    void                    Initialize();
    glm::ivec3              GetPositionFromCenterPosition(const glm::vec3& centerPosition);
    bool                    DoesRayIntersectCube(const glm::vec3& rayStart, const glm::vec3& rayDirection, const glm::vec3& cubeMin, const glm::vec3& cubeMax);
    glm::ivec3              GetTargetBlockPosition(const glm::vec3& rayStart, const glm::vec3& rayEnd);
    std::vector<glm::ivec3> GetIntersectedBlockPositions(const glm::vec3& rayStart, const glm::vec3& rayEnd);
};

} // namespace tlr