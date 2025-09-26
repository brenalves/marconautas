#include "app.h"

App *App::s_instance = nullptr;

App::App()
    : m_running(true), m_isLogged(false)
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwDefaultWindowHints();

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    m_window = glfwCreateWindow(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, "Marconautas", nullptr, nullptr);
    if (!m_window)
    {
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(m_window);

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
        App::getInstance()->close();
    });

    m_gui = new GUI(m_window);

    m_gui->setLoginSubmitCallback([](char* username, char* errorPtr) -> bool {
        // make connection with broker via mqtt
        // if success, set m_isLogged variable true to change page and return true;
        // if error, return false
    });
}

App::~App()
{
    delete m_gui;

    glfwDestroyWindow(m_window);
}

void App::run()
{
    while (m_running)
    {
        glfwPollEvents();

        m_gui->beginRender();
        ImGuiIO &io = ImGui::GetIO();
        ImGuiViewport *viewport = ImGui::GetMainViewport();

        // Set up fullscreen window
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags host_window_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_MenuBar;

        // Remove window padding so DockSpace uses full area
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        // Begin the invisible host window
        if(ImGui::Begin("MainDockSpaceHost", nullptr, host_window_flags))
        {
            ImGui::PopStyleVar();
    
            // Create the dockspace
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    
            if(!m_isLogged)
                m_gui->renderLogin();

            // if(ImGui::Begin("Active Users"))
            // {
            //     for(int i = 0; i < 10; i++)
            //     {
            //         ImGui::Text("User %d", i);
            //     }
            // }
            // ImGui::End();

            // if(ImGui::Begin("Options"))
            // {
            //     if(ImGui::Button("Disconnect"))
            //     {
            //         close();
            //     }
            // }
            // ImGui::End();

            // if(ImGui::Begin("Marcohub"))
            // {
            //     ImGui::Text("MarcoHub");
            // }
            // ImGui::End();
        }
        ImGui::End();

        glClear(GL_COLOR_BUFFER_BIT);

        m_gui->endRender();

        glfwSwapBuffers(m_window);
    }
}

void App::close()
{
    m_running = false;
}

App *App::getInstance()
{
    if (!s_instance)
        s_instance = new App();

    return s_instance;
}
