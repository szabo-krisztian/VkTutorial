#pragma once

#include <glm/glm.hpp>

namespace tlr
{
    
class Block
{
public:
    static constexpr glm::vec3 CORNER_OFFSET{1, 1, 1};

    void       Initialize(const glm::ivec3& position);
    glm::ivec3 GetPosition() const;
    glm::vec3  GetColor() const;
    glm::vec3  GetCenter() const;
    bool       IsPlaced() const;
    void       Place();
    void       Break();
    glm::mat4  GetModelMatrix() const;

private:
    glm::ivec3 _position;
    glm::vec3  _color;
    bool       _isPlaced = false;
    bool       _isInitialized = false;

    glm::vec3 GetRandomVec3();
};

} // namespace tlr