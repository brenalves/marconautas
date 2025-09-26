#include "gui.h"

#include <iostream>

GUI::GUI(GLFWwindow *window)
    : m_loginStatus(NOPE), m_loginTextField("")
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // ini file at root folder
    io.IniFilename = "../ui.ini";

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

GUI::~GUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


void GUI::beginRender()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::endRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::renderLogin()
{
    if(m_loginStatus == FAIL && (glfwGetTime() - m_loginFailTime > LOGIN_FAIL_TIME_FADE_OUT))
        m_loginStatus = NOPE;

    if(ImGui::Begin("Login"))
    {
        ImGui::Text("Bem vindo ao Marcomundo");
        ImGui::Text("Your username:");
        ImGui::SameLine();
        ImGui::InputText("##inputLogin", m_loginTextField, sizeof(m_loginTextField));
        if(ImGui::Button("Entrar"))
        {
            if(!m_loginCallback(m_loginTextField, m_loginError))
            {
                m_loginStatus = FAIL;
                m_loginFailTime = glfwGetTime();
            }
        }

        if(m_loginStatus == FAIL)
        {
            ImGui::Text("%s", m_loginError);
        }
    }
    ImGui::End();
}
