#pragma once
#include <string_view>
#include <SDL.h>

constexpr std::string_view TITLE = "quadtree";
constexpr int WINDOW_X = SDL_WINDOWPOS_CENTERED;
constexpr int WINDOW_Y = SDL_WINDOWPOS_CENTERED;
constexpr int WINDOW_W = 800;
constexpr int WINDOW_H = 600;
constexpr Uint32 WINDOW_SDL_FLAGS = SDL_WINDOW_RESIZABLE;
constexpr Uint32 RENDERER_SDL_FLAGS = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
constexpr std::string_view PATH_TO_FONT = "../assets/arial.ttf";
constexpr size_t FONT_SIZE = 25;
constexpr SDL_Rect dstRect = SDL_Rect{10, 10, 450, 75};
constexpr double TARGET_FPS = 30.0;
constexpr double FRAME_MS   = 1000.0 / TARGET_FPS;
constexpr float PI = 3.14159265358979323846264338327950288f;

// World space units
constexpr int PIXEL_TO_UNIT_CONVERSION = 100; // i.e. 100 pixels = 1 world unit