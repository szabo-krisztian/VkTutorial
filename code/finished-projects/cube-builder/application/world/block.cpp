#include "block.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace tlr
{

void Block::Initialize(const glm::ivec3& position)
{
    assert(!_isInitialized && "position cannot be changed after initialization!");
    _position = position;
}

glm::ivec3 Block::GetPosition() const
{
    return _position;
}

glm::vec3 Block::GetCenter() const
{
    return static_cast<glm::vec3>(_position) + CORNER_OFFSET / 2.0f;
}

bool Block::IsPlaced() const
{
    return _isPlaced;
}

void Block::Place()
{
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