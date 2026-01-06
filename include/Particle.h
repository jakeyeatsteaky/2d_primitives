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

template <size_t N, class T>
void create_render_buffer_with_arc(const std::array<T, N> &arc, std::vector<T> &drawBuffer, auto rad, auto increment)
{
    if (N == 0)
        return;
    // derive the center point from a tangent point on the circle and the radius
    // have to make an assumption that the first point is theta == 0
    // :: sine(theta) == 0
    // :: cos(theta) == 1
    auto centerX = arc[0].x - rad;
    auto centerY = arc[0].y;
    for (auto i = 0; i < arc.size(); i++)
    {
        T point = arc.at(i);
        auto negX = centerX + (centerX - point.x);
        auto negY = centerY + (centerY - point.y);
        drawBuffer.emplace_back(T{negX, negY});    // Q2
        drawBuffer.emplace_back(T{negX, point.y}); // Q3
        drawBuffer.emplace_back(T{point.x, negY}); //

        for (auto x = centerX; x <= point.x; x+= increment)
        {
            for (auto y = centerY; y <= point.y; y+= increment)
            {
                auto negFillX = centerX + (centerX - x);
                auto negFillY = centerY + (centerY - y);
                drawBuffer.emplace_back(T{x, y});        // filler point
                drawBuffer.emplace_back(T{negFillX, y}); //
                drawBuffer.emplace_back(T{negFillX, negFillY});
                drawBuffer.emplace_back(T{x, negFillY});
            }
        }
    }
    
}


// world space render test

struct world_point {
    float x;
    float y;
};

struct __Particle {
    float x;
    float y;
    float r;
    std::vector<world_point> render_data{};
};

std::ostream& operator<<(std::ostream &out, const __Particle & particle);

inline void produce_render_data(__Particle& particle) {
   // quarter arc
   particle.render_data.clear();
   std::array<world_point, 100> quarter_arc{};
    size_t numSamples = 100;
    double radStep = ((PI / 2) / numSamples);
    size_t idx = 0;
    for (double angleInRad = 0.0; angleInRad < (PI / 2); angleInRad += radStep)
    {
        auto arcX = static_cast<float>(particle.x + particle.r * SDL_cos(angleInRad)); // quarter circle rad = pi/2
        auto arcY = static_cast<float>(particle.y + particle.r * SDL_sin(angleInRad)); // quarter circle rad = pi/2

        quarter_arc[idx++] = world_point{arcX, arcY}; 
    }

    create_render_buffer_with_arc<100ul, world_point>(quarter_arc, particle.render_data, particle.r, 0.01);

    std::cout << particle << std::endl;
}

std::ostream& operator<<(std::ostream &out, const __Particle & particle) {
    out << "Particle:\n";
    out << "\tx: " << particle.x;
    out << "\n\ty: " << particle.y;
    out << "\n\tr: " << particle.r << std::endl;
    return out;
}