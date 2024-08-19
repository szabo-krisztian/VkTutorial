#pragma once

#include <iostream>

#include <glm/glm.hpp>

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

    World();

    Block& operator[](glm::ivec3 position);

private:
    Block _world[X_DIMENSION * 2 + 1][X_DIMENSION * 2 + 1][X_DIMENSION * 2 + 1];

    void Initialize();
};

} // namespace tlr