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
        instance->_pressedKeys.insert(key);

        auto it = instance->_keyPressListeners.find(key);
        if (it != instance->_keyPressListeners.end())
        {
            for (const auto& func : it->second)
            {
                func();
            }
        }
    }
    else if (action == GLFW_RELEASE)
    {
        instance->_pressedKeys.erase(key);

        auto it = instance->_keyReleaseListeners.find(key);
        if (it != instance->_keyReleaseListeners.end())
        {
            for (const auto& func : it->second)
            {
                func();
            }
        }
    }
}

void InputManager::GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    InputManager* instance = GetInstance();
    for (const auto& func : instance->_cursorPositionListeners)
    {
        func(xpos, ypos);
    }
}

void InputManager::AddCursorPositionListener(FunctionWrapper<void(double, double)> listener)
{
    _cursorPositionListeners.insert(listener);
}

void InputManager::RemoveCursorPositionListener(FunctionWrapper<void(double, double)> listener)
{
    _cursorPositionListeners.erase(listener);
}

void InputManager::AddKeyPressListener(int keyCode, FunctionWrapper<void()> listener)
{
    _keyPressListeners[keyCode].insert(listener);
}

void InputManager::RemoveKeyPressListener(int keyCode, FunctionWrapper<void()> listener)
{
    auto& listeners = _keyPressListeners[keyCode];
    listeners.erase(listener);
    if (listeners.empty())
    {
        _keyPressListeners.erase(keyCode);
    }
}

void InputManager::AddKeyReleaseListener(int keyCode, FunctionWrapper<void()> listener)
{
    _keyReleaseListeners[keyCode].insert(listener);
}

void InputManager::RemoveKeyReleaseListener(int keyCode, FunctionWrapper<void()> listener)
{
    auto& listeners = _keyReleaseListeners[keyCode];
    listeners.erase(listener);
    if (listeners.empty())
    {
        _keyReleaseListeners.erase(keyCode);
    }
}

void InputManager::AddKeyHoldListener(int keyCode, FunctionWrapper<void()> listener)
{
    _keyHoldListeners[keyCode].insert(listener);
}

void InputManager::RemoveKeyHoldListener(int keyCode, FunctionWrapper<void()> listener)
{
    auto& listeners = _keyHoldListeners[keyCode];
    listeners.erase(listener);
    if (listeners.empty())
    {
        _keyHoldListeners.erase(keyCode);
    }
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
        auto it = _keyHoldListeners.find(key);
        if (it != _keyHoldListeners.end())
        {
            for (const auto& func : it->second)
            {
                func();
            }
        }
    }
}

} // namespace tlr