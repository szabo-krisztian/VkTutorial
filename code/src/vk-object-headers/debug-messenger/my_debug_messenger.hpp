#pragma once

#include "my_instance.hpp"

#include <stdexcept>
#include <iostream>

namespace tlr
{

class MyDebugMessenger
{
public:
    MyDebugMessenger(MyInstance& instance);
    ~MyDebugMessenger();

    MyDebugMessenger(const MyDebugMessenger&) = delete;
    MyDebugMessenger operator=(const MyDebugMessenger&) = delete;

private:
    MyInstance&                        mInstance;
    VkDebugUtilsMessengerEXT           mDebugMessenger;
    VkDebugUtilsMessengerCreateInfoEXT mDebugMessengerCreateInfo;

    void SetVkDebugUtilsMessengerCreateInfoEXT();
    void InitDebugMessengerInstance();
};

} // namespace tlr