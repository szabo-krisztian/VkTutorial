#include "timer.hpp"

namespace tlr
{

Timer::Timer()
{
    _lastTime = std::chrono::high_resolution_clock::now();
}

void Timer::Update()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = currentTime - _lastTime;
    _deltaTime = duration.count();
    _elapsedTime += _deltaTime;
    _lastTime = currentTime;
}

float Timer::GetDeltaTime() const
{
    return _deltaTime;
}

float Timer::GetElapsedTime() const
{
    return _elapsedTime;
}

} // namespace tlr
