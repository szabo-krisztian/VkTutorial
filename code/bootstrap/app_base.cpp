/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#include "app_base.hpp"

#include "toolset.hpp"
#include "instance_builder.hpp"
#include "physical_device_selector.hpp"
#include "device_builder.hpp"
#include "swapchain_builder.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (_deletionQueue).PushFunction(lambda)

namespace tlr
{

CameraCreateInfo cameraCI = {
    glm::vec3(0.0f, 0.0f, -40.0f),  // initialPosition
    glm::vec3(0.0f, 1.0f, 0.0f),   // worldUp
    glm::radians(45.0f),           // fov
    800.0f / 600.0f,               // aspect
    90.0f,                         // initialYaw
    90.0f,                         // initialPitch
    0.1f,                          // sensitivity
    8.5f,                          // movementSpeed
    0.1f,                          // near
    100.0f                         // far
};

AppBase::AppBase() : camera(cameraCI)
{
    Init();
}

AppBase::~AppBase()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
}

void AppBase::Run()
{
    isAppRunning = true;

    while (!glfwWindowShouldClose(window) && isAppRunning)
    {
        glfwPollEvents();
        inputManager->Update();
        timer.Update();
        
        Update();
    }
}

void AppBase::ExitApp()
{
    isAppRunning = false;
}

void AppBase::Init()
{
    InitGLFW();
    InitVulkan();
    InitSwapchain();
    InitInputManager();
}

void AppBase::InitGLFW()
{
    glfwInit();
    ENQUEUE_OBJ_DEL(( []() { glfwTerminate(); } ));

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
    ENQUEUE_OBJ_DEL(( [this]() { glfwDestroyWindow(window); } ));
}

void AppBase::InitVulkan()
{
    InstanceBuilder builder;
    Instance instances = builder.SetApplicationName("Vulkan app")
                                .SetApplicationVersion(VK_MAKE_VERSION(1, 0,0 ))
                                .SetEngineName("No engine")
                                .SetEngineVersion(VK_MAKE_VERSION(1, 0, 0))
                                .SetApiVersion(VK_MAKE_VERSION(1, 1, 0))
                                .RequestDefaultLayers()
                                .RequestDefaultExtensions()
                                .UseDebugMessenger()
                                .Build();
    instance = instances.instance;
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyInstance(instance, nullptr); } ));

    debugMessenger = instances.debugMessenger;
    ENQUEUE_OBJ_DEL(( [this]() { DestroyDebugMessenger(instance, debugMessenger); } ));

    VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroySurfaceKHR(instance, surface, nullptr); } ));

    PhysicalDeviceSelector physicalDeviceSelector{instance, surface};
    physicalDevice = physicalDeviceSelector.EnableDedicatedGPU()
                                           .Select();
    
    DeviceBuilder deviceBuilder{physicalDevice};
    device = deviceBuilder.EnableValidationLayers()
                          .Build();
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDevice(device, nullptr); } ));
}

void AppBase::InitSwapchain()
{
    SwapchainBuilder builder(window, surface, physicalDevice, device);
    swapchain = builder.SetDesiredFormat(VK_FORMAT_B8G8R8A8_SRGB)
                       .SetDesiredColorSpace(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                       .SetDesiredPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
                       .SetDesiredExtent(WINDOW_WIDTH, WINDOW_HEIGHT)
                       .SetDesiredImageCount(physicalDevice.swapchainSupportDetails.capabilities.minImageCount + 1)
                       .SetDesiredArrayLayerCount(1)
                       .SetImageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                       .Build();
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroySwapchainKHR(device, swapchain, nullptr); } ));
    for (auto& imageView : swapchain.imageViews)
    {
        ENQUEUE_OBJ_DEL(( [&]() { vkDestroyImageView(device, imageView, nullptr); } ));
    }
}

void AppBase::InitInputManager()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    InputManager::Init(window);
    inputManager = InputManager::GetInstance();
    
    inputManager->AddKeyPressListener(GLFW_KEY_ESCAPE, [&]() { ExitApp(); });
    
    inputManager->AddCursorPositionListener([&](float xoffset, float yoffset) { camera.CursorMovementCallback(xoffset, yoffset); });
    
    inputManager->AddKeyHoldListener(GLFW_KEY_W,          [&]() { camera.MoveForward(timer.GetDeltaTime());  });
    inputManager->AddKeyHoldListener(GLFW_KEY_A,          [&]() { camera.MoveLeft(timer.GetDeltaTime());     });
    inputManager->AddKeyHoldListener(GLFW_KEY_S,          [&]() { camera.MoveBackward(timer.GetDeltaTime()); });
    inputManager->AddKeyHoldListener(GLFW_KEY_D,          [&]() { camera.MoveRight(timer.GetDeltaTime());    });
    inputManager->AddKeyHoldListener(GLFW_KEY_SPACE,      [&]() { camera.MoveUp(timer.GetDeltaTime());       });
    inputManager->AddKeyHoldListener(GLFW_KEY_LEFT_SHIFT, [&]() { camera.MoveDown(timer.GetDeltaTime());     });
}

} // namespace tlr