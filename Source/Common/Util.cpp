#include "Util.h"

std::chrono::steady_clock::time_point Util::lastTime = std::chrono::high_resolution_clock::now();

std::string Util::FormatWindowTitle(const std::string &subTitle)
{
    return fmt::format("{} | {} | {}",
        APP_NAME, APP_VERSION, subTitle);
}

float Util::DeltaTime()
{
    std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
    lastTime = currentTime;
    return deltaTime;
}
