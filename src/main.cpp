#include <SDL.h>
#include <SDL_render.h>
#include <iostream>
#include <vector>
#include <ranges>
#include <array>

#include "util.h"

#define _mu [[maybe_unused]]
#define SDL_MAIN_USE_CALLBACKS 1 // only sdl3 


/**
 * TODO
 * - [ ] frame rate font info
 * - [x] render_circle filled
 * - [x] mouse input
 * - [ ] might be time to optimize drawing. check framerate slowdown w/ current drawing algo
 *          see if you can make it better
 */

struct Color {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
};
template<size_t N>
struct Circle_ {
    SDL_Point center;
    int radius;
    std::array<SDL_Point,N> buffer_{};
};

using Circle = Circle_<100>;

constexpr std::string_view TITLE = "quadtree";
constexpr int WINDOW_X = SDL_WINDOWPOS_CENTERED;
constexpr int WINDOW_Y = SDL_WINDOWPOS_CENTERED;
constexpr int WINDOW_W = 800;
constexpr int WINDOW_H = 600;
constexpr Uint32 WINDOW_SDL_FLAGS = SDL_WINDOW_RESIZABLE;
constexpr Uint32 RENDERER_SDL_FLAGS = SDL_RENDERER_ACCELERATED;

constexpr float PI = 3.14159265358979323846264338327950288f;

static SDL_Window* gWindow = nullptr;
static SDL_Renderer* gRenderer = nullptr;
static bool gRunning = true;
static std::vector<SDL_Point> gPoints = {};
static std::vector<Circle> gCircles{};

constexpr auto addCircleCallback_ = [](int x, int y)->void{
    gCircles.push_back(Circle{
        .center = {
            .x = x,
            .y = y
        },
        .radius = 25,
        .buffer_{}
    });
    std::cout << "Added a circle, size now: " << gCircles.size() << std::endl;
};

constexpr auto produceQuarterArc_ = [](auto &circle) -> void {
    size_t numSamples = circle.buffer_.size();
    double radStep = ((PI / 2) / numSamples);
    size_t idx = 0;
    for (double angleInRad = 0.0; angleInRad < (PI / 2); angleInRad += radStep)
    {
        auto arcX = static_cast<int>(circle.center.x + circle.radius * SDL_cos(angleInRad)); // quarter circle rad = pi/2
        auto arcY = static_cast<int>(circle.center.y + circle.radius * SDL_sin(angleInRad)); // quarter circle rad = pi/2

        circle.buffer_[idx++] = SDL_Point{arcX, arcY};
    }
};

void processInput();
void update();
void draw();
void render_circle(int x, int y);
void render_circle(const Circle& circle);
void handleKeyboardEvent(SDL_KeyboardEvent event);
void handleMouseButtonEvent(SDL_MouseButtonEvent event);

template<size_t N>
void fill_circle(const std::array<SDL_Point, N>& arc, std::vector<SDL_Point>& drawBuffer, int rad);

int main(_mu int argc, _mu char **argv)
{
    gPoints.reserve(400);
    std::cout << gPoints.capacity() << std::endl;

    for(int i = 0; i < gPoints.capacity(); i++) {
        SDL_Point pt = {
            .x = i,
            .y = i
        };
        gPoints.emplace_back(std::move(pt));
    }
    
    SDL_Point center = {
        .x = 500,
        .y = 500
    };

    int radius = 50; //px?

    // loop quarter circle, add all 4 points to the buffer
    constexpr size_t numSamples = 100;
    double radStep = ((PI/2)/ numSamples);
    std::array<SDL_Point, numSamples> arc{};
    size_t idx = 0;
    for (double angleInRad = 0.0; angleInRad < (PI/2); angleInRad+=radStep) {
        auto arcX = static_cast<int>(center.x + radius * SDL_cos(angleInRad)); // quarter circle rad = pi/2
        auto arcY = static_cast<int>(center.y + radius * SDL_sin(angleInRad)); // quarter circle rad = pi/2

        arc[idx++] = SDL_Point{arcX, arcY};
    }

    // fill circle
    fill_circle<numSamples>(arc, gPoints, radius);
    
    

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL" << std::endl;
        return -1;
    }

    gWindow = SDL_CreateWindow(
        TITLE.data(),
        WINDOW_X,
        WINDOW_Y,
        WINDOW_W,
        WINDOW_H,
        WINDOW_SDL_FLAGS);

    if (!gWindow)
    {
        std::cerr << "Error creating SDL window" << std::endl;
        SDL_Quit();
        return -1;
    }

    gRenderer = SDL_CreateRenderer(
        gWindow,
        -1,
        RENDERER_SDL_FLAGS
    );

    if (!gRenderer) {
        std::cerr << "Error creating SDL renderer" << std::endl;
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        return -1;
    }

    while(gRunning) {
       processInput();
       update();
       draw(); 
    }

    std::cout << "hello world" << std::endl;

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow((gWindow));
    SDL_Quit();
    return 0;
}

void processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) {
                gRunning = SDL_FALSE;
            }

            if (event.type == SDL_KEYDOWN) {
                handleKeyboardEvent(event.key);
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                handleMouseButtonEvent(event.button);
            }
            
        }
}

void update() {
    // update logic and positions
    const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
    /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    const float red = (float) (0.5 + 0.5 * SDL_sin(now));
    const float green = (float) (0.5 + 0.5 * SDL_sin(now + PI * 2 / 3));
    const float blue = (float) (0.5 + 0.5 * SDL_sin(now + PI * 4 / 3));
    
    const Uint8 r = static_cast<Uint8>(red*255);
    const Uint8 g = static_cast<Uint8>(green*255);
    const Uint8 b = static_cast<Uint8>(blue*255);
    SDL_SetRenderDrawColor(gRenderer, r, g, b, SDL_ALPHA_OPAQUE);  /* new color, full alpha. */
}

void draw() {
    SDL_RenderClear(gRenderer);
    Color previousColor = {};
    SDL_GetRenderDrawColor(gRenderer, &previousColor.r, &previousColor.g, &previousColor.b, &previousColor.a);

    // render vector of points
    SDL_SetRenderDrawColor(gRenderer, previousColor.g, previousColor.b, previousColor.r, previousColor.a);
    
    // render circles in circle container'
    std::ranges::for_each(gCircles, produceQuarterArc_); // not sure how this would be different than forEach w/o ranges
    for (const auto& circle : gCircles) {
        render_circle(circle); // right now, this is just taking a circleobject, and using its arcbuffer to push the filled pixel coords into the gPoints array
    }

    SDL_RenderDrawPoints(gRenderer, gPoints.data(), gPoints.size());

    SDL_SetRenderDrawColor(gRenderer, previousColor.r, previousColor.g, previousColor.b, previousColor.a);
    SDL_RenderPresent(gRenderer);
}

void handleKeyboardEvent(SDL_KeyboardEvent event) {
    if (event.type == SDL_KEYDOWN) {
        if (event.keysym.sym == SDLK_ESCAPE) {
            gRunning = false;
        }
    }
}

void handleMouseButtonEvent(SDL_MouseButtonEvent button) {
    std::cout << "clicking the app at: " << button.x << ", " << button.y << std::endl;
    addCircleCallback_(button.x, button.y);
}

void render_circle(const Circle& circle) {
   fill_circle(circle.buffer_, gPoints, circle.radius); 
}

void render_circle(int x, int y) {
    // SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);


}

template<size_t N>
void fill_circle(const std::array<SDL_Point, N>& arc, std::vector<SDL_Point>& drawBuffer, int rad) {
    if (N == 0) return;
    // derive the center point from a tangent point on the circle and the radius 
    // have to make an assumption that the first point is theta == 0
    // :: sine(theta) == 0
    // :: cos(theta) == 1
    int centerX = arc[0].x - rad;
    int centerY = arc[0].y;
    for (int i = 0; i < arc.size(); i++) {
        SDL_Point point = arc.at(i);
        int negX = centerX + (centerX - point.x);
        int negY = centerY + (centerY - point.y);
        drawBuffer.emplace_back(SDL_Point{negX, negY}); // Q2
        drawBuffer.emplace_back(SDL_Point{negX,point.y}); // Q3
        drawBuffer.emplace_back(SDL_Point{point.x, negY}); // 

        for(int x = centerX; x <= point.x; x++) {
            for (int y = centerY; y <= point.y; y++) {
                int negFillX = centerX + (centerX - x);
                int negFillY = centerY + (centerY - y);
                drawBuffer.emplace_back(SDL_Point{x,y});// filler point
                drawBuffer.emplace_back(SDL_Point{negFillX,y});// 
                drawBuffer.emplace_back(SDL_Point{negFillX,negFillY});
                drawBuffer.emplace_back(SDL_Point{x,negFillY});
            }
        }
    }
}