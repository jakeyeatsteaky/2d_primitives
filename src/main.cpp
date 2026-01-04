#include <SDL.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ranges>
#include <array>
#include <queue>

#include "util.h"

#define _mu [[maybe_unused]]
#define SDL_MAIN_USE_CALLBACKS 1 // only sdl3

/**
 * TODO
 * - [ ] might be time to refactor soon
 *      - [ ] clean up file structure
 *      - [ ] clean up application structure (globals etc)
 *      - [ ] 
 * - [ ] In order to apply movement I have to decouple movement from pizel space
 *          it needs to be an arbitrary unit within screen space, then converted to
 *          pixel coordinates.  i dont know hwo to do this yet
 * - [x] render num circles
 * - [x] frame rate font info
 * - [x] render_circle filled
 * - [x] mouse input
 * - [x] might be time to optimize drawing. check framerate slowdown w/ current drawing algo
 *          see if you can make it better
 */

struct Color
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
};
template <size_t N>
struct Circle_
{
    SDL_Point center;
    int radius;
    std::array<SDL_Point, N> buffer_{};
};

using Circle = Circle_<100>;

constexpr std::string_view TITLE = "quadtree";
constexpr int WINDOW_X = SDL_WINDOWPOS_CENTERED;
constexpr int WINDOW_Y = SDL_WINDOWPOS_CENTERED;
constexpr int WINDOW_W = 800;
constexpr int WINDOW_H = 600;
constexpr Uint32 WINDOW_SDL_FLAGS = SDL_WINDOW_RESIZABLE;
constexpr Uint32 RENDERER_SDL_FLAGS = SDL_RENDERER_ACCELERATED;
constexpr std::string_view PATH_TO_FONT = "../assets/arial.ttf";
constexpr size_t FONT_SIZE = 25;
constexpr SDL_Rect dstRect = SDL_Rect{10, 10, 450, 75};
constexpr double TARGET_FPS = 300.0;
constexpr double FRAME_MS   = 1000.0 / TARGET_FPS;

constexpr float PI = 3.14159265358979323846264338327950288f;

static SDL_Window *gWindow = nullptr;
static SDL_Renderer *gRenderer = nullptr;
static TTF_Font *gFont = nullptr;
static SDL_Texture *gFontTexture = nullptr;
static bool gRunning = true;
static std::vector<SDL_Point> gPoints = {};
static std::vector<SDL_Point> gTest = {};
static std::queue<Circle> gCircles{};
static size_t gNumCircles = 0;
static double gDeltaTime = 0;
static Circle gTestCircle = {
    .center.x = 100,
    .center.y = 100,
    .radius = 10,
    .buffer_ = {}
};

constexpr auto produceQuarterArc_ = [](auto &circle) -> void
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

constexpr auto addCircleCallback_ = [](int x, int y) -> void
{
    Circle add = {
        .center = {
            .x = x,
            .y = y},
        .radius = 25,
        .buffer_ = {}};

    produceQuarterArc_(add);

    gCircles.push(add);
    ++gNumCircles;
};

constexpr auto updateFPS_ = [](double fps) -> void
{
    char buffer[256]; // Ensure buffer is large enough for the string
    std::sprintf(buffer, "FPS: %.2f    DT: %.5f    %d particles", fps, gDeltaTime, gNumCircles);

    SDL_Surface *fontSurface = TTF_RenderText_Solid(gFont, buffer, SDL_Color{255, 255, 255, 255});
    gFontTexture = SDL_CreateTextureFromSurface(gRenderer, fontSurface);
    SDL_FreeSurface(fontSurface);
};

void processInput();
void update();
void draw();
void render_circle(int x, int y);
void render_circle(const Circle &circle);
void handleKeyboardEvent(SDL_KeyboardEvent event);
void handleMouseButtonEvent(SDL_MouseButtonEvent event);

template <size_t N>
void add_circle_to_buffer(const std::array<SDL_Point, N> &arc, std::vector<SDL_Point> &drawBuffer, int rad);

int main(_mu int argc, _mu char **argv)
{
    gPoints.reserve(400);
    

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (TTF_Init() != 0)
    {
        std::cerr << "Error initializing TTF: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    gFont = TTF_OpenFont(PATH_TO_FONT.data(), FONT_SIZE);

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
        RENDERER_SDL_FLAGS);

    if (!gRenderer)
    {
        std::cerr << "Error creating SDL renderer" << std::endl;
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        return -1;
    }

    updateFPS_(10.0);

Uint64 start = SDL_GetTicks64();
double nextFrame = start + FRAME_MS;
Uint64 lastFrame = start;
Uint32 frames = 0;
Uint64 fpsTimer = start;

while (gRunning) {
    Uint64 now = SDL_GetTicks64();

    if (now < nextFrame) {
        SDL_Delay(static_cast<Uint32>(nextFrame - now));
        now = SDL_GetTicks64();
    }

    gDeltaTime = (now - lastFrame) / 1000.0;
    lastFrame = now;

    processInput();
    update();
    draw();

    frames++;
    nextFrame += FRAME_MS;

    if (now - fpsTimer >= 2000) {
        double fps = frames * 1000.0 / (now - fpsTimer);
        updateFPS_(fps);
        frames = 0;
        fpsTimer = now;
    }
}

    std::cout << "hello world" << std::endl;

    SDL_DestroyTexture(gFontTexture);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow((gWindow));
    TTF_CloseFont(gFont);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

void processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            gRunning = SDL_FALSE;
        }

        if (event.type == SDL_KEYDOWN)
        {
            handleKeyboardEvent(event.key);
        }

        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            handleMouseButtonEvent(event.button);
        }
    }
}

void update()
{
    // update logic and positions
    const double now = ((double)SDL_GetTicks()) / 1000.0; /* convert from milliseconds to seconds. */
    /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    const float red = (float)(0.5 + 0.5 * SDL_sin(now));
    const float green = (float)(0.5 + 0.5 * SDL_sin(now + PI * 2 / 3));
    const float blue = (float)(0.5 + 0.5 * SDL_sin(now + PI * 4 / 3));

    const Uint8 r = static_cast<Uint8>(red * 255);
    const Uint8 g = static_cast<Uint8>(green * 255);
    const Uint8 b = static_cast<Uint8>(blue * 255);
    SDL_SetRenderDrawColor(gRenderer, r, g, b, SDL_ALPHA_OPAQUE); /* new color, full alpha. */

    while (!gCircles.empty())
    {
        const auto &circle = gCircles.front();
        add_circle_to_buffer(circle.buffer_, gPoints, circle.radius);
        gCircles.pop();
    }
}

void draw()
{
    SDL_RenderClear(gRenderer);
    Color previousColor = {};
    SDL_GetRenderDrawColor(gRenderer, &previousColor.r, &previousColor.g, &previousColor.b, &previousColor.a);

    // render vector of points
    SDL_SetRenderDrawColor(gRenderer, previousColor.g, previousColor.b, previousColor.r, previousColor.a);
    SDL_RenderDrawPoints(gRenderer, gPoints.data(), gPoints.size());

    SDL_SetRenderDrawColor(gRenderer, previousColor.r, previousColor.g, previousColor.b, previousColor.a);

    SDL_RenderCopy(gRenderer, gFontTexture, 0, &dstRect);

    SDL_SetRenderDrawColor(gRenderer,255,255,255,255);
    SDL_RenderDrawPoints(gRenderer, gTest.data(), gTest.size());
    SDL_RenderPresent(gRenderer);
}

void handleKeyboardEvent(SDL_KeyboardEvent event)
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.keysym.sym == SDLK_ESCAPE)
        {
            gRunning = false;
        }
    }
}

void handleMouseButtonEvent(SDL_MouseButtonEvent button)
{
    std::cout << "clicking the app at: " << button.x << ", " << button.y << std::endl;
    addCircleCallback_(button.x, button.y);
    std::cout << "gPoints.size() = " << gPoints.size() << std::endl;
}

void render_circle(const Circle &circle)
{
    add_circle_to_buffer(circle.buffer_, gPoints, circle.radius);
}

void render_circle(int x, int y)
{
    // SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
}

template <size_t N>
void add_circle_to_buffer(const std::array<SDL_Point, N> &arc, std::vector<SDL_Point> &drawBuffer, int rad)
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