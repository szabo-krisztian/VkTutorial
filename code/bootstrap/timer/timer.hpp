#pragma once

#include <chrono>

namespace tlr
{

class Timer
{
public:
    Timer();

    void Update();
    float GetElapsedTime() const;
    float GetDeltaTime() const;

private:
    std::chrono::high_resolution_clock::time_point _lastTime;
    float _deltaTime = 0.0f;
    float _elapsedTime = 0.0f;
};

} // namespace tlr
