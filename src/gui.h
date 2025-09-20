#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

class GUI
{
public:
    GUI(GLFWwindow* window);
    ~GUI();

    void beginRender();
    void endRender();
};