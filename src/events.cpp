#include <SDL.h>
#include <functional>
#include <iostream>

#include "events.h"



void handleKeyboardEvent(SDL_KeyboardEvent event, bool& running) {
    if (event.type == SDL_KEYDOWN)  {
        if (event.keysym.sym == SDLK_ESCAPE) {
            running = false;
        }
    }
}

void handleMouseButtonEvent(SDL_MouseButtonEvent button, MouseEventCallback cb_) {
    std::cout << "clicking the app at: " << button.x << ", " << button.y << std::endl;
    cb_(button.x, button.y);
}

