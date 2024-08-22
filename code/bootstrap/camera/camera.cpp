#include "camera.hpp"
#include <iostream>

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

glm::vec3 Camera::GetForwardVector() const
{
    return _front;
}

glm::vec3 Camera::GetPosition() const
{
    return _position;
}

void Camera::MoveForward(float deltaTime)
{
    glm::vec3 front_y_null(_front.x, 0.0f, _front.z);
    _position += glm::normalize(front_y_null) * deltaTime * _movementSpeed;
}

void Camera::MoveBackward(float deltaTime)
{
    glm::vec3 front_y_null(_front.x, 0.0f, _front.z);
    _position -= glm::normalize(front_y_null) * deltaTime * _movementSpeed;
}

void Camera::MoveRight(float deltaTime)
{
    _position += _right * deltaTime * _movementSpeed;
}

void Camera::MoveLeft(float deltaTime)
{
    _position -= _right * deltaTime * _movementSpeed;
}

void Camera::MoveUp(float deltaTime)
{
    _position += _worldUp * deltaTime * _movementSpeed;
}

void Camera::MoveDown(float deltaTime)
{
    _position -= _worldUp * deltaTime * _movementSpeed;
}

void Camera::CursorMovementCallback(float xoffset, float yoffset)
{    
    _yaw -= xoffset * _sensitivity;
    _pitch -= yoffset * _sensitivity;
    _pitch = glm::clamp(_pitch, 1.0f, 179.0f);
    UpdateDirections();
}

void Camera::SetPosition(const glm::vec3& position)
{
    _position = position;
}

void Camera::SetDirection(const glm::vec3& direction)
{
    _front = direction;
}

void Camera::SetLookAtPoint(const glm::vec3& lookAt)
{
    glm::vec3 newDirecton = lookAt - _position;
    _front = glm::normalize(newDirecton);
}

void Camera::SetMovementSpeed(float speed)
{
    _movementSpeed = speed;
}

void Camera::UpdateDirections()
{
    glm::vec3 front;
    front.x = sin(glm::radians(_pitch)) * cos(glm::radians(_yaw));
    front.y = cos(glm::radians(_pitch));
    front.z = sin(glm::radians(_pitch)) * sin(glm::radians(_yaw));
    
    _front = glm::normalize(front);
    _right = glm::normalize(glm::cross(_front, -_worldUp));
    _up = glm::normalize(glm::cross(_right, _front));
}

} // namespace tlr