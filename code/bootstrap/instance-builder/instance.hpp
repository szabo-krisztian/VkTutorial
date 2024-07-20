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

namespace tlr
{

struct Instance
{
    VkInstance               instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    operator VkInstance() const
    {
        return instance;
    }
};

} // namespace tlr