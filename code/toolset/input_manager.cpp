#include "input_manager.hpp"

namespace tlr
{

GLFWwindow* InputManager::_window = nullptr;

void InputManager::Init(GLFWwindow* window)
{
    assert(_window == nullptr && "InputManager already initialized!");
    _window = window;
    glfwSetKeyCallback(_window, GLFWKeyCallback);
    glfwSetCursorPosCallback(_window, GLFWCursorCallback);
}

InputManager* InputManager::GetInstance()
{
    static InputManager instance;
    return &instance;
}

void InputManager::GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    InputManager* instance = GetInstance();
    
    if (action == GLFW_PRESS)
    {
        auto it = instance->_keyPressed.find(key);
        if (it != instance->_keyPressed.end())
        {
            instance->_keyPressed[key].Raise();
        }
    }
    else if (action == GLFW_RELEASE)
    {
        auto it = instance->_keyReleased.find(key);
        if (it != instance->_keyReleased.end())
        {
            instance->_keyReleased[key].Raise();
        }
    }
}

void InputManager::GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    InputManager* instance = GetInstance();
    instance->_cursorMoved.Raise(xpos, ypos);
}

void InputManager::AddCursorPositionListener(std::function<void(double, double)>&& listener)
{
    _cursorMoved += FunctionWrapper<void(double, double)>(std::move(listener));
}

void InputManager::RemoveCursorPositionListener(std::function<void(double, double)>&& listener)
{
    _cursorMoved -= FunctionWrapper<void(double, double)>(std::move(listener));
}

void InputManager::AddKeyPressListener(int keyCode, std::function<void()>&& listener)
{
    _keyPressed[keyCode] += FunctionWrapper<void()>(std::move(listener));
}

void InputManager::RemoveKeyPressListener(int keyCode, std::function<void()>&& listener)
{
    _keyPressed[keyCode] -= FunctionWrapper<void()>(std::move(listener));
}

void InputManager::AddKeyReleaseListener(int keyCode, std::function<void()>&& listener)
{
    _keyReleased[keyCode] += FunctionWrapper<void()>(std::move(listener));
}

void InputManager::RemoveKeyReleaseListener(int keyCode, std::function<void()>&& listener)
{
    _keyReleased[keyCode] -= FunctionWrapper<void()>(std::move(listener));
}

void InputManager::AddKeyHoldListener(int keyCode, std::function<void()>&& listener)
{
    _keyIsBeingPressed[keyCode] += FunctionWrapper<void()>(std::move(listener));
}

void InputManager::RemoveKeyHoldListener(int keyCode, std::function<void()>&& listener)
{
    _keyIsBeingPressed[keyCode] += FunctionWrapper<void()>(std::move(listener));
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
        auto it = _keyIsBeingPressed.find(key);
        if (it != _keyIsBeingPressed.end())
        {
            _keyIsBeingPressed[key].Raise();
        }
    }
}

} // namespace tlr