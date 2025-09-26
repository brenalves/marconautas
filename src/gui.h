#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <functional>

#define LOGIN_FAIL_TIME_FADE_OUT 3

enum LoginState
{
    NOPE,
    SUCCESS,
    FAIL
};

class GUI
{
public:
    GUI(GLFWwindow *window);
    ~GUI();

    void beginRender();
    void endRender();

    void renderLogin();

    inline void setLoginSubmitCallback(std::function<bool(char *username, char *error)> f) { m_loginCallback = f; }

private:
    char m_loginTextField[32];
    LoginState m_loginStatus;
    double m_loginFailTime;
    char m_loginError[128];
    std::function<bool(char *, char *error)> m_loginCallback;
};