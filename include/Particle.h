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

template <size_t N>
void create_render_buffer_with_arc(const std::array<SDL_Point, N> &arc, std::vector<SDL_Point> &drawBuffer, int rad)
{
    if (N == 0)
        return;
    // derive the center point from a tangent point on the circle and the radius
    // have to make an assumption that the first point is theta == 0
    // :: sine(theta) == 0
    // :: cos(theta) == 1
    int centerX = arc[0].x - rad;
    int centerY = arc[0].y;
    for (int i = 0; i < arc.size(); i++)
    {
        SDL_Point point = arc.at(i);
        int negX = centerX + (centerX - point.x);
        int negY = centerY + (centerY - point.y);
        drawBuffer.emplace_back(SDL_Point{negX, negY});    // Q2
        drawBuffer.emplace_back(SDL_Point{negX, point.y}); // Q3
        drawBuffer.emplace_back(SDL_Point{point.x, negY}); //

        for (int x = centerX; x <= point.x; x++)
        {
            for (int y = centerY; y <= point.y; y++)
            {
                int negFillX = centerX + (centerX - x);
                int negFillY = centerY + (centerY - y);
                drawBuffer.emplace_back(SDL_Point{x, y});        // filler point
                drawBuffer.emplace_back(SDL_Point{negFillX, y}); //
                drawBuffer.emplace_back(SDL_Point{negFillX, negFillY});
                drawBuffer.emplace_back(SDL_Point{x, negFillY});
            }
        }
    }
}