#include "App.h"


void App::init() {
    m_initialized = true;
    return;
}

bool App::initSuccess() {
    return m_initialized;
}