#include "camera.hpp"

namespace tlr
{

Camera::Camera(const CameraCreateInfo& createInfo) : 
    _position(createInfo.initialPosition),
    _worldUp(createInfo.worldUp),
    _fov(createInfo.fov),
    _aspect(createInfo.aspect),
    _yaw(createInfo.initialYaw),
    _pitch(createInfo.initialPitch),
    _sensitivity(createInfo.sensitivity),
    _movementSpeed(createInfo.movementSpeed),
    _near(createInfo.near),
    _far(createInfo.far)
{
    UpdateDirections();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(_position, _position + _front, _up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return glm::perspective(_fov, _aspect, _near, _far);
}

glm::mat4 Camera::GetViewProjectionMatrix() const
{
    return GetProjectionMatrix() * GetViewMatrix();
}

void Camera::MoveForward(float deltaTime)
{
    _position += _front * deltaTime * _movementSpeed;
}

void Camera::MoveBackward(float deltaTime)
{
    _position -= _front * deltaTime * _movementSpeed;
}

void Camera::MoveRight(float deltaTime)
{
    _position += _right * deltaTime * _movementSpeed;
}

void Camera::MoveLeft(float deltaTime)
{
    _position -= _right * deltaTime * _movementSpeed;
}

void Camera::CursorMovementCallback(float xoffset, float yoffset)
{
    _yaw += xoffset * _sensitivity;
    _pitch += yoffset * _sensitivity;
    _pitch = glm::clamp(_pitch, 1.0f, 179.0f);
    UpdateDirections();
}

void Camera::UpdateDirections()
{
    glm::vec3 front;
    front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    front.y = sin(glm::radians(_pitch));
    front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    
    _front = glm::normalize(front);
    _right = glm::normalize(glm::cross(_front, _worldUp));  
    _up = glm::normalize(glm::cross(_right, _front));
}

} // namespace tlr