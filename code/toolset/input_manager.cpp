#include "input_manager.hpp"

namespace std
{
    /*
     * std::function<T> types cannot be hashed by deafult
     */

    template <typename T>
    std::size_t hash<std::function<T>>::operator()(const std::function<T>& f) const
    {
        // This hash function is very naive and might not be suitable for production code.
        // It's just for demonstration purposes.
        return std::hash<std::uintptr_t>{}(reinterpret_cast<std::uintptr_t>(&f));
    }

    template <typename T>
    bool equal_to<std::function<T>>::operator()(const std::function<T>& lhs, const std::function<T>& rhs) const
    {
        // Function comparison using function pointers is not directly possible.
        // This is just a naive approach for demonstration.
        return &lhs == &rhs;
    }

} // namespace std

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

    void InputManager::AddCursorPositionListener(std::function<void(double, double)> listener)
    {
        _cursorPositionListeners.insert(listener);
    }

    void InputManager::RemoveCursorPositionListener(std::function<void(double, double)> listener)
    {
        _cursorPositionListeners.erase(listener);
    }

    void InputManager::AddKeyPressListener(int keyCode, std::function<void()> listener)
    {
        _keyPressListeners[keyCode].insert(listener);
    }

    void InputManager::RemoveKeyPressListener(int keyCode, std::function<void()> listener)
    {
        auto& listeners = _keyPressListeners[keyCode];
        listeners.erase(listener);
        if (listeners.empty())
        {
            _keyPressListeners.erase(keyCode);
        }
    }

    void InputManager::AddKeyReleaseListener(int keyCode, std::function<void()> listener)
    {
        _keyReleaseListeners[keyCode].insert(listener);
    }

    void InputManager::RemoveKeyReleaseListener(int keyCode, std::function<void()> listener)
    {
        auto& listeners = _keyReleaseListeners[keyCode];
        listeners.erase(listener);
        if (listeners.empty())
        {
            _keyReleaseListeners.erase(keyCode);
        }
    }

    void InputManager::AddKeyHoldListener(int keyCode, std::function<void()> listener)
    {
        _keyHoldListeners[keyCode].insert(listener);
    }

    void InputManager::RemoveKeyHoldListener(int keyCode, std::function<void()> listener)
    {
        auto& listeners = _keyHoldListeners[keyCode];
        listeners.erase(listener);
        if (listeners.empty())
        {
            _keyHoldListeners.erase(keyCode);
        }
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

    bool InputManager::IsKeyPressed(int keyCode)
    {
        auto it = _pressedKeys.find(keyCode);
        return it != _pressedKeys.end();
    }

} // namespace tlr
