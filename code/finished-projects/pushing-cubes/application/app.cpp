#include "app.hpp"
#include "initializers.hpp"
#include "toolset.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (_deletionQueue).PushFunction(lambda)

namespace tlr
{

CameraCreateInfo cameraCI = {
    glm::vec3(0.0f, 0.0f, 0.0f),   // initialPosition
    glm::vec3(0.0f, -1.0f, 0.0f),  // worldUp
    glm::radians(45.0f),           // fov
    800.0f / 600.0f,               // aspect
    90.0f,                         // initialYaw
    90.0f,                         // initialPitch
    0.1f,                          // sensitivity
    2.5f,                          // movementSpeed
    0.1f,                          // near
    100.0f                         // far
};

App::App() : _camera(cameraCI)
{
    inputManager->AddCursorPositionListener(std::bind(&Camera::CursorMovementCallback, _camera, std::placeholders::_1, std::placeholders::_2));
}

App::~App()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
}

void App::Update()
{
    // Update logic
}

} // namespace tlr
