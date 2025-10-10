#pragma once

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <functional>
#include <vector>

class Window
{
public:
    Window(int width, int height, const char* title);
    ~Window();

    inline void setOnCloseButtonClick(const std::function<void()>& callback) { _onCloseButtonClick = callback; }
    inline void setOnLoginButtonSubmit(const std::function<void(const char*)>& callback) { _onLoginButtonSubmit = callback; }
    inline void setActiveUsersVector(std::vector<std::string>* users) { _activeUsers = users; }

    void beginFrame();
    void showLogin();
    void showMain();
    void endFrame();

    void setTitle(const char* title);

private:
    GLFWwindow* _window;
    int _width;
    int _height;
    const char* _title;

    char _username[64];

    std::function<void()> _onCloseButtonClick;
    std::function<void(const char*)> _onLoginButtonSubmit;

    std::vector<std::string>* _activeUsers;
};