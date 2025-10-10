#include "window.h"

Window::Window(int width, int height, const char *title)
    : _width(width), _height(height), _title(title)
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwDefaultWindowHints();

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    _window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!_window)
    {
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(_window);
    glfwSetWindowUserPointer(_window, this);
    glfwSwapInterval(1); // Enable vsync

    glfwSetWindowCloseCallback(_window, [](GLFWwindow *window)
                               {
        Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        win->_onCloseButtonClick(); });

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // ini file at root folder
    io.IniFilename = "../ui.ini";

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(_window, true); // Second param install
    ImGui_ImplOpenGL3_Init();

    _username[0] = '\0';
}

Window::~Window()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Window::beginFrame()
{
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::showLogin()
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    window_flags |= ImGuiWindowFlags_NoBackground; // Optional: make it fully transparent

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("DockSpaceHost", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    // Create the dockspace
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    ImGui::End();

    if (ImGui::Begin("Login"))
    {
        ImGui::Text("Bem vindo ao Marcomundo");
        ImGui::Text("Your username:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        ImGui::InputText("##inputLogin", _username, sizeof(_username));
        if (ImGui::Button("Entrar"))
        {
            if (_onLoginButtonSubmit)
                _onLoginButtonSubmit(_username);
        }

        ImGui::End();
    }
}

void Window::showMain()
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("DockSpaceHost", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::Begin("Active Users"))
    {
        for (const auto &user : *_activeUsers)
        {
            ImGui::Text("%s", user.c_str());
        }

        ImGui::End();
    }

    if (ImGui::Begin("Options"))
    {
        if (ImGui::Button("Logout"))
        {
            if (_onCloseButtonClick)
                _onCloseButtonClick();
        }

        ImGui::End();
    }

    if (ImGui::Begin("Chats"))
    {
        ImGui::Text("Chats will be here.");
        ImGui::End();
    }
}

void Window::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_window);
}

void Window::setTitle(const char *title)
{
    _title = title;
    glfwSetWindowTitle(_window, title);
}
