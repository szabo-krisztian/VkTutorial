#include "block.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <iostream>

namespace tlr
{

void Block::Initialize(const glm::ivec3& position)
{
    assert(!_isInitialized && "position cannot be changed after initialization!");
    _position = position;
    _color = {1.0f, 0.0f, 0.0f};
}

glm::ivec3 Block::GetPosition() const
{
    return _position;
}

glm::vec3 Block::GetColor() const
{
    return _color;
}

glm::vec3 Block::GetCenter() const
{
    return static_cast<glm::vec3>(_position) + CORNER_OFFSET / 2.0f;
}

bool Block::IsPlaced() const
{
    return _isPlaced;
}

glm::vec3 Block::GetRandomVec3()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    float x = dis(gen);
    float y = dis(gen);
    float z = dis(gen);

    return glm::vec3(x, y, z);
}

void Block::Place()
{
    _color = GetRandomVec3();
    _isPlaced = true;
}

void Block::Break()
{
    _isPlaced = false;
}

glm::mat4 Block::GetModelMatrix() const
{
    return glm::translate(glm::mat4(1.0f), static_cast<glm::vec3>(_position));
}

} // namespace tlr