#include "Timer.h"

#include "Common/Logger.h"

Timer::Timer()
{
    lastFrameTime = std::chrono::high_resolution_clock::now();
}

float Timer::DeltaTime()
{
    std::chrono::high_resolution_clock::time_point currentFrameTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - lastFrameTime).count();

    lastFrameTime = currentFrameTime;

    return deltaTime;
}
