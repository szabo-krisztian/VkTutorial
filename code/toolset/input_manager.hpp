#pragma once

#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <cassert>
#include <memory>
#include <iostream>

#include <GLFW/glfw3.h>

namespace tlr
{

template<typename T>
class FunctionWrapper;

template<typename R, typename... Args>
class FunctionWrapper<R(Args...)>
{
public:
    FunctionWrapper() = default;

    FunctionWrapper(std::function<R(Args...)> func) : _func(std::move(func)) {}

    bool operator==(const FunctionWrapper<R(Args...)>& other) const
    {
        return _func.target_type() == other._func.target_type();
    }

    R operator()(Args... args) const
    {
        return _func(std::forward<Args>(args)...);
    }

    std::size_t hash() const
    {
        return _func.target_type().hash_code();
    }

private:
    std::function<R(Args...)> _func;
};

} // namespace tlr

namespace std
{
    template<typename R, typename... Args>
    struct hash<tlr::FunctionWrapper<R(Args...)>>
    {
        std::size_t operator()(const tlr::FunctionWrapper<R(Args...)>& wrapper) const
        {
            return wrapper.hash();
        }
    };
} // namespace std

namespace tlr
{

class InputManager
{
public:
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    static void Init(GLFWwindow* window);
    static InputManager* GetInstance();

    static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos);

    void AddCursorPositionListener(FunctionWrapper<void(double, double)> listener);
    void RemoveCursorPositionListener(FunctionWrapper<void(double, double)> listener);

    void AddKeyPressListener(int keyCode, FunctionWrapper<void()> listener);
    void RemoveKeyPressListener(int keyCode, FunctionWrapper<void()> listener);

    void AddKeyReleaseListener(int keyCode, FunctionWrapper<void()> listener);
    void RemoveKeyReleaseListener(int keyCode, FunctionWrapper<void()> listener);

    void AddKeyHoldListener(int keyCode, FunctionWrapper<void()> listener);
    void RemoveKeyHoldListener(int keyCode, FunctionWrapper<void()> listener);

    bool IsKeyPressed(int keyCode);

    void Update();

private:
    InputManager() = default;

    static GLFWwindow* _window;
    std::unordered_map<int, std::unordered_set<FunctionWrapper<void()>>> _keyPressListeners;
    std::unordered_map<int, std::unordered_set<FunctionWrapper<void()>>> _keyReleaseListeners;
    std::unordered_map<int, std::unordered_set<FunctionWrapper<void()>>> _keyHoldListeners;
    std::unordered_set<FunctionWrapper<void(double, double)>> _cursorPositionListeners;
    std::unordered_set<int> _pressedKeys;
};

} // namespace tlr