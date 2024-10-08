#include "input_manager.hpp"

namespace tlr
{

bool InputManager::_isInitialized = false;

void InputManager::Init(GLFWwindow* window)
{
    assert(!_isInitialized && "InputManager already initialized!");
    _isInitialized = true;
    glfwSetKeyCallback(window, GLFWKeyboardButtonCallback);
    glfwSetMouseButtonCallback(window, GLFWMouseButtonCallback);
    glfwSetCursorPosCallback(window, GLFWCursorCallback);
}

InputManager* InputManager::GetInstance()
{
    static InputManager instance;
    return &instance;
}

void InputManager::GLFWKeyboardButtonCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    InputManager* instance = GetInstance();
    
    switch (action)
    {
    case GLFW_PRESS:
        instance->_keyPressed[key].Raise();
        instance->_pressedKeys.insert(key);
        break;

    case GLFW_RELEASE:
        instance->_keyReleased[key].Raise();
        instance->_pressedKeys.erase(key);
        break;
    }
}

void InputManager::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    InputManager* instance = GetInstance();
    
    switch (action)
    {
    case GLFW_PRESS:
        instance->_keyPressed[button].Raise();
        instance->_pressedKeys.insert(button);
        break;

    case GLFW_RELEASE:
        instance->_keyReleased[button].Raise();
        instance->_pressedKeys.erase(button);
        break;
    }
}

void InputManager::GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    static bool first = true;
    static double firstXpos = xpos;
    static double firstYpos = ypos;

    if (first)
    {
        firstXpos = xpos;
        firstYpos = ypos;
        first = false;
    }

    float deltaX = static_cast<float>(xpos - firstXpos);
    float deltaY = static_cast<float>(ypos - firstYpos);

    firstXpos = xpos;
    firstYpos = ypos;

    InputManager* instance = GetInstance();
    instance->_cursorMoved.Raise(deltaX, -deltaY);
}

void InputManager::AddCursorPositionListener(std::function<void(float, float)>&& listener)
{
    _cursorMoved += std::move(listener);
}

void InputManager::RemoveCursorPositionListener(std::function<void(float, float)>&& listener)
{
    _cursorMoved -= std::move(listener);
}

void InputManager::AddKeyPressListener(int keyCode, std::function<void()>&& listener)
{
    _keyPressed[keyCode] += std::move(listener);
}

void InputManager::RemoveKeyPressListener(int keyCode, std::function<void()>&& listener)
{
    _keyPressed[keyCode] -= std::move(listener);
}

void InputManager::AddKeyReleaseListener(int keyCode, std::function<void()>&& listener)
{
    _keyReleased[keyCode] += std::move(listener);
}

void InputManager::RemoveKeyReleaseListener(int keyCode, std::function<void()>&& listener)
{
    _keyReleased[keyCode] -= std::move(listener);
}

void InputManager::AddKeyHoldListener(int keyCode, std::function<void()>&& listener)
{
    _keyIsBeingPressed[keyCode] += std::move(listener);
}

void InputManager::RemoveKeyHoldListener(int keyCode, std::function<void()>&& listener)
{
    _keyIsBeingPressed[keyCode] -= std::move(listener);
}

bool InputManager::IsKeyPressed(int keyCode)
{
    auto it = _pressedKeys.find(keyCode);
    return it != _pressedKeys.end();
}

void InputManager::Update()
{
    for (int key : _pressedKeys)
    {
        _keyIsBeingPressed[key].Raise();
    }
}

} // namespace tlr