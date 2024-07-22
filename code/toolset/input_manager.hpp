#pragma once

#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <cassert>
#include <memory>
#include <iostream>

#include <GLFW/glfw3.h>

namespace std
{
    template <typename T>
    struct hash<std::function<T>>
    {
        std::size_t operator()(const std::function<T>& f) const;
    };

    template <typename T>
    struct equal_to<std::function<T>>
    {
        bool operator()(const std::function<T>& lhs, const std::function<T>& rhs) const;
    };
}

namespace tlr
{

class InputManager
{
public:
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    static void Init(GLFWwindow* window);
    static InputManager* GetInstance(); // Return pointer to InputManager
    static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos);

    void AddCursorPositionListener(std::function<void(double, double)> listener);
    void RemoveCursorPositionListener(std::function<void(double, double)> listener);

    void AddKeyPressListener(int keyCode, std::function<void()> listener);
    void RemoveKeyPressListener(int keyCode, std::function<void()> listener);

    void AddKeyReleaseListener(int keyCode, std::function<void()> listener);
    void RemoveKeyReleaseListener(int keyCode, std::function<void()> listener);

    void AddKeyHoldListener(int keyCode, std::function<void()> listener);
    void RemoveKeyHoldListener(int keyCode, std::function<void()> listener);

    bool IsKeyPressed(int keyCode);

    void Update();

private:
    InputManager() = default;

    static GLFWwindow* _window;
    std::unordered_map<int, std::unordered_set<std::function<void()>>> _keyPressListeners;
    std::unordered_map<int, std::unordered_set<std::function<void()>>> _keyReleaseListeners;
    std::unordered_map<int, std::unordered_set<std::function<void()>>> _keyHoldListeners;
    std::unordered_set<std::function<void(double, double)>> _cursorPositionListeners;
    std::unordered_set<int> _pressedKeys;
};

} // namespace tlr
