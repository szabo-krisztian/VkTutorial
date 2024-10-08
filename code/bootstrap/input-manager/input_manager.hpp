#pragma once

#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <cassert>
#include <memory>
#include <iostream>

#include <GLFW/glfw3.h>

#include "event.hpp"

namespace tlr
{

class InputManager
{
public:
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    static void Init(GLFWwindow* window);
    static InputManager* GetInstance();

    void AddCursorPositionListener(std::function<void(float, float)>&& listener);
    void RemoveCursorPositionListener(std::function<void(float, float)>&& listener);

    void AddKeyPressListener(int keyCode, std::function<void()>&& listener);
    void RemoveKeyPressListener(int keyCode, std::function<void()>&& listener);

    void AddKeyReleaseListener(int keyCode, std::function<void()>&& listener);
    void RemoveKeyReleaseListener(int keyCode, std::function<void()>&& listener);

    void AddKeyHoldListener(int keyCode, std::function<void()>&& listener);
    void RemoveKeyHoldListener(int keyCode, std::function<void()>&& listener);

    bool IsKeyPressed(int keyCode);

    void Update();

private:
    static bool _isInitialized;

    static void GLFWKeyboardButtonCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos);

    Event<float, float>              _cursorMoved;
    std::unordered_set<int>          _pressedKeys;
    std::unordered_map<int, Event<>> _keyPressed;
    std::unordered_map<int, Event<>> _keyReleased;
    std::unordered_map<int, Event<>> _keyIsBeingPressed;

    InputManager() = default;
};

} // namespace tlr