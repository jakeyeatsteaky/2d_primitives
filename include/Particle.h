#pragma once
#include <stddef.h>
#include <array>

#include <SDL.h>


// Old particle render method
template <size_t N>
struct Particle_ {
    SDL_Point center;
    int radius;
    std::array<SDL_Point, N> buffer_{};
};

using Particle = Particle_<100>;

constexpr auto produce_quarter_arc_ = [](auto &circle) -> void
{
    size_t numSamples = circle.buffer_.size();
    std::memset(circle.buffer_.data(), 0, circle.buffer_.size());
    double radStep = ((PI / 2) / numSamples);
    size_t idx = 0;
    for (double angleInRad = 0.0; angleInRad < (PI / 2); angleInRad += radStep)
    {
        auto arcX = static_cast<int>(circle.center.x + circle.radius * SDL_cos(angleInRad)); // quarter circle rad = pi/2
        auto arcY = static_cast<int>(circle.center.y + circle.radius * SDL_sin(angleInRad)); // quarter circle rad = pi/2

        circle.buffer_[idx++] = SDL_Point{arcX, arcY};
    }
};