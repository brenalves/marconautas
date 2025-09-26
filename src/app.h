#pragma once

#include <memory>
#include <GLFW/glfw3.h>
#include <stdexcept>

#include "gui.h"

#define WINDOW_DEFAULT_WIDTH 800
#define WINDOW_DEFAULT_HEIGHT 600

class App
{
private:
    App();

public:
    ~App();

    void run();

    void close();

    static App* getInstance();

private:
    bool m_running;
    bool m_isLogged;

    GLFWwindow* m_window;

    GUI* m_gui;

    static App* s_instance;
};