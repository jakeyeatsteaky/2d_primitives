
#pragma once

#include <atomic>

class App {
    std::atomic<bool> m_initialized;

public:
    App(): m_initialized(false) {}
    void init();
    bool initSuccess();


};