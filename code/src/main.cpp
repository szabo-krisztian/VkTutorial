#include <iostream>


/*
#include "instance.hpp"
#include "debug_messenger.hpp"
#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "app.hpp"
*/

#include "bootstrap.hpp"

int main()
{
    glfwInit();

    /*
    try
    {
        tlr::Instance instance;
        tlr::DebugMessenger debugMessenger;
        
        tlr::Window window;
        tlr::Device device;
        tlr::Swapchain swapchain;
        tlr::App app;
        

        while (window.IsWindowActive())
        {
            glfwPollEvents();
            //app.DrawFrame();
        }
        
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    */    

   
    try
    {
        tlr::InstanceBuilder builder;
        builder.SetApplicationName("Vulkan app")
               .SetApplicationVersion(VK_MAKE_VERSION(1, 0,0 ))
               .SetEngineName("No engine")
               .SetEngineVersion(VK_MAKE_VERSION(1, 0, 0))
               .SetApiVersion(VK_MAKE_VERSION(1, 1, 0))
               .RequestDefaultLayers()
               .RequestDefaultExtensions()
               .UseDebugMessenger()
               .Build();
        
        VkInstance instance = builder.GetInstance();
        VkDebugUtilsMessengerEXT debugMessenger = builder.GetMessengerInstance();

        tlr::DestroyDebugMessenger(instance, debugMessenger);
        vkDestroyInstance(instance, nullptr);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}