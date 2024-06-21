#pragma once

#include "state_board.hpp"

#include <stdexcept>
#include <iostream>

namespace tlr
{

class DebugMessenger
{
public:
    DebugMessenger();
    ~DebugMessenger();

    DebugMessenger(const DebugMessenger&) = delete;
    DebugMessenger operator=(const DebugMessenger&) = delete;

private:
    VkDebugUtilsMessengerEXT           mDebugMessenger;
    VkDebugUtilsMessengerCreateInfoEXT mDebugMessengerCreateInfo;

    void SetVkDebugUtilsMessengerCreateInfoEXT();
    void InitDebugMessengerInstance();
};

} // namespace tlr