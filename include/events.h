
#pragma once

#include <SDL.h>
#include <stdio.h>

using MouseEventCallback = std::function<void(int, int)>;

void handleKeyboardEvent(SDL_KeyboardEvent event, bool& running);
void handleMouseButtonEvent(SDL_MouseButtonEvent event, MouseEventCallback cb_);