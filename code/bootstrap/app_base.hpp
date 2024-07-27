/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "physical_device.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "input_manager.hpp"
#include "camera.hpp"
#include "deletion_queue.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (deletionQueue).PushFunction(lambda)

namespace tlr
{

class AppBase
{
public:
    AppBase();
    ~AppBase();

    void Run();
    void ExitApp();

protected:
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    VkInstance               instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    GLFWwindow               *window;
    VkSurfaceKHR             surface;
    PhysicalDevice           physicalDevice;
    Device                   device;
    Swapchain                swapchain;
    InputManager             *inputManager;
    Camera                   camera;
    DeletionQueue            deletionQueue;

    bool isAppRunning = false;

    virtual void Update() = 0;

private:
    void Init();
    void InitGLFW();
    void InitVulkan();
    void InitSwapchain();
    void InitInputManager();
};

} // namespace tlr
