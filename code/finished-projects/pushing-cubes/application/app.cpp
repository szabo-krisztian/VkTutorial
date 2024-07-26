#include "app.hpp"

#include "initializers.hpp"
#include "toolset.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (_deletionQueue).PushFunction(lambda)

namespace tlr
{

App::App()
{
    
}

App::~App()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
}

void App::Update()
{

}

} // namespace tlr