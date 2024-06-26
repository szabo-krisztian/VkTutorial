#pragma once

#include "state_board.hpp"
#include "iwindow.h"

#include <stdexcept>

namespace tlr
{

class Window : public IWindow
{
public:
    Window();
    ~Window();

    Window(const Window&) = delete;
    Window operator=(const Window&) = delete;

    const VkSurfaceKHR& GetSurface()     const override;
          GLFWwindow*   GetWindow()      const override;
          bool          IsWindowActive() const override;

private:
    const uint32_t M_WIDTH = 800;
    const uint32_t M_HEIGHT = 600;

    VkSurfaceKHR   mSurface;
    GLFWwindow*    mWindow;

    void InitWindow();
};

} // namespace tlr