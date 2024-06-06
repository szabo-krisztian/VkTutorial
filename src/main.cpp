#include "my_instance.hpp"
#include "my_device.hpp"
#include "my_window.hpp"
#include "my_swap_chain.hpp"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <optional>

int main()
{
    try
    {
        tlr::MyVulkanInstance instance;
        tlr::MyVulkanWindow window(instance);
        tlr::MyVulkanDevice device(instance, window);
        tlr::MyVulkanSwapChain swapChain(window, device);

        while (window.IsWindowActive())
        {
            glfwPollEvents();
        }
        
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}