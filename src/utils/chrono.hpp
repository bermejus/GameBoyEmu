#pragma once

#include <chrono>

class Chrono
{
private:
    std::chrono::steady_clock::time_point start;

public:
    Chrono() : start(std::chrono::steady_clock::now()) {}

    void reset()
    {
        start = std::chrono::steady_clock::now();
    }

    float elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() / 1000000.0f;
    }
};