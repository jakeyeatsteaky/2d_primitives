#include <SDL.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ranges>
#include <array>
#include <queue>

#include "util.h"
#include "events.h"
#include "constants.h"
#include "Particle.h"

#define _mu [[maybe_unused]]

/**
 * TODO
 * - [ ] might be time to refactor soon
 *      - [ ] clean up file structure
 *      - [ ] clean up application structure (globals etc)
 *      - [ ] 
 * - [ ] In order to apply movement I have to decouple movement from pizel space
 *          it needs to be an arbitrary unit within screen space, then converted to
 *          pixel coordinates.  i dont know hwo to do this yet
 *      - [ ] figure out a better way to render particles
 *              scanline method
 * 
 * - [x] render num circles
 * - [x] frame rate font info
 * - [x] render_circle filled
 * - [x] mouse input
 * - [x] might be time to optimize drawing. check framerate slowdown w/ current drawing algo
 *          see if you can make it better
 * - [ ] update FPS counter logic
 *      - [ ] using chrono utilities instead of SDL_GetTicks
 *      - [ ] Logic of fps should be decoupled.  there is update and gametick frame rate, and then render framerate
 *              render rate should be on vsync (pass in vsynC flag)
 *              the update logic is separate
 */

static SDL_Window *gWindow = nullptr;
static SDL_Renderer *gRenderer = nullptr;
static TTF_Font *gFont = nullptr;
static SDL_Texture *gFontTexture = nullptr;
static bool gRunning = true;
static std::vector<SDL_Point> gPoints = {};
static std::queue<Particle> gCircles{};
static std::vector<SDL_Point> point_data{};
static int gNumCircles = 0;
static double gDeltaTime = 0;

// new particle rendering method
static std::vector<__Particle> gParticleVec{};

constexpr auto add_particle_ = [](int x, int y) -> void
{
    Particle add = {
        .center = {
            .x = x,
            .y = y},
        .radius = 25,
        .buffer_ = {}};

    produce_quarter_arc_(add);

    gCircles.push(add);
    ++gNumCircles;
};

constexpr auto update_fps_ = [](double fps) -> void {
    char buffer[256]; // Ensure buffer is large enough for the string
    std::snprintf(buffer, sizeof(buffer), "FPS: %.2f    DT: %.5f    %d particles", fps, gDeltaTime, gNumCircles);

    SDL_Surface *fontSurface = TTF_RenderText_Solid(gFont, buffer, SDL_Color{255, 255, 255, 255});
    gFontTexture = SDL_CreateTextureFromSurface(gRenderer, fontSurface);
    SDL_FreeSurface(fontSurface);
};

int init();
void processInput();
void update();
void draw();


float get_world_space(int pixelSpaceValue);
int get_pixel_space(float worldSpaceValue);

template <size_t N>
void add_circle_to_buffer(const std::array<SDL_Point, N> &arc, std::vector<SDL_Point> &drawBuffer, int rad);

int main(_mu int argc, _mu char **argv)
{
    gPoints.reserve(400);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (TTF_Init() != 0) {
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

    if (!gWindow) {
        std::cerr << "Error creating SDL window" << std::endl;
        SDL_Quit();
        return -1;
    }

    gRenderer = SDL_CreateRenderer(
        gWindow,
        -1,
        RENDERER_SDL_FLAGS);

    if (!gRenderer) {
        std::cerr << "Error creating SDL renderer" << std::endl;
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        return -1;
    }


    // init new particle method
    __Particle particle = {
        .x = get_world_space(100), // pixel space
        .y = get_world_space(100), // pixel space
        .r = get_world_space(25)
    };
    std::cout << particle << std::endl;

    produce_render_data(particle);
    gParticleVec.push_back(particle);

    for (const auto& p : gParticleVec) {
        
        // point_data.reserve(p.render_data.size());
        // std::transform(p.render_data.begin(), p.render_data.end(), point_data.begin(), apply_);
        for (auto i = 0; i < p.render_data.size(); i++) {
            int x = get_pixel_space(p.render_data[i].x);
            int y = get_pixel_space(p.render_data[i].y);
            point_data.push_back(SDL_Point{x,y});
        }
    }


    update_fps_(TARGET_FPS);

    auto start = SDL_GetTicks64();
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
            update_fps_(fps);
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

void processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            gRunning = SDL_FALSE;
        }

        if (event.type == SDL_KEYDOWN) {
            handleKeyboardEvent(event.key, gRunning);
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            handleMouseButtonEvent(event.button, add_particle_);
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
        create_render_buffer_with_arc(circle.buffer_, gPoints, circle.radius, 1);
        gCircles.pop();
    }

    auto& particle = gParticleVec.at(0);
    particle.x += get_world_space(100) * gDeltaTime;
    particle.y += get_world_space(100) * gDeltaTime;
    // point_data.clear();
    produce_render_data(particle);
    point_data.clear();
    for(const auto [x,y] : particle.render_data) {
        point_data.push_back(SDL_Point{.x = get_pixel_space(x), .y = get_pixel_space(y)});
    }
}

const auto apply_ = [](world_point wp) -> SDL_Point {
    SDL_Point ret {
        .x = 0,
        .y = 0
    };
    
    ret.x = get_pixel_space(wp.x);
    ret.y = get_pixel_space(wp.y);
    return ret;
};

void draw()
{
    SDL_RenderClear(gRenderer);
    Uint8 previousColor_r, previousColor_g, previousColor_b, previousColor_a;
    SDL_GetRenderDrawColor(gRenderer, &previousColor_r, &previousColor_g, &previousColor_b, &previousColor_a);

    // render vector of points
    SDL_SetRenderDrawColor(gRenderer, previousColor_g, previousColor_b, previousColor_r, previousColor_a);
    SDL_RenderDrawPoints(gRenderer, gPoints.data(), gPoints.size());

    // test world space
    
    SDL_RenderDrawPoints(gRenderer, point_data.data(), point_data.size());

    SDL_SetRenderDrawColor(gRenderer, previousColor_r, previousColor_g, previousColor_b, previousColor_a);

    SDL_RenderCopy(gRenderer, gFontTexture, 0, &dstRect);

    SDL_RenderPresent(gRenderer);
}


float get_world_space(int pixelSpaceValue) {
    if (pixelSpaceValue <= 0) return 0.0f;
    float worldSpace = static_cast<float>(pixelSpaceValue) / PIXEL_TO_UNIT_CONVERSION;
    return worldSpace;
}

int get_pixel_space(float worldSpace) {
    if (worldSpace <= 0.0f) return 0;
    return worldSpace * PIXEL_TO_UNIT_CONVERSION;
}