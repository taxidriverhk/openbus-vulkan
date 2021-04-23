#pragma once

#include <chrono>

class Timer
{
public:
    Timer();

    float DeltaTime();

private:
    std::chrono::high_resolution_clock::time_point lastFrameTime;
};
