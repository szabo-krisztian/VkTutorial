#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace tlr
{

struct CameraCreateInfo
{
    glm::vec3 initialPosition;
    glm::vec3 worldUp;
    float fov;
    float aspect;
    float initialYaw;
    float initialPitch;
    float sensitivity;
    float movementSpeed;
    float near;
    float far;
};

class Camera
{
public:
    Camera(const CameraCreateInfo& createInfo);

    glm::mat4 GetViewMatrix()           const;
    glm::mat4 GetProjectionMatrix()     const;
    glm::mat4 GetViewProjectionMatrix() const;
    glm::vec3 GetForwardVector()        const;
    glm::vec3 GetPosition()             const;

    void CursorMovementCallback(float xoffset, float yoffset);
    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveRight(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveUp(float deltaTime);
    void MoveDown(float deltaTime);

private:
    glm::vec3 _position;
    glm::vec3 _worldUp;
    float _fov;
    float _aspect;
    float _yaw;
    float _pitch;
    float _sensitivity;
    float _movementSpeed;
    float _near;
    float _far;

    glm::vec3 _front;
    glm::vec3 _right;
    glm::vec3 _up;

    void UpdateDirections();
};

} // namespace tlr