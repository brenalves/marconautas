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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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

    // Set window icon
    GLFWimage icon;
    icon.pixels = stbi_load("images/icon.png", &icon.width, &icon.height, 0, 4);
    if (icon.pixels)
    {
        glfwSetWindowIcon(_window, 1, &icon);
        stbi_image_free(icon.pixels);
    }

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
    _groupName[0] = '\0';
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
    // Centered login modal-like window (non-movable, non-resizable)
    const ImVec2 win_size(500.f, 125.f);
    const ImVec2 win_pos((float)_width * 0.5f - win_size.x * 0.5f,
                         (float)_height * 0.5f - win_size.y * 0.5f);
    ImGui::SetNextWindowPos(win_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
    if (ImGui::Begin("Login", nullptr, flags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::Text("Bem vindo ao Marcomundo");
        ImGui::Spacing();

        ImGui::Text("Seu nome de usuário:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        bool submitted = ImGui::InputText("##username", _username, sizeof(_username), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (ImGui::Button("Entrar") || submitted)
        {
            if (_onLoginButtonSubmit && _username[0] != '\0')
                _onLoginButtonSubmit(_username);
        }

        // Break lines
        ImGui::NewLine();
        ImGui::NewLine();

        ImGui::TextUnformatted("Marconautas © 2025 - Todos os direitos reservados.");

        ImGui::End();
    }
}

void Window::showMain()
{
    // Layout metrics
    const float left_w = 320.f;
    const float groups_h = 260.f;
    const float options_h = 120.f;
    const float spacing = 6.f;
    const ImGuiWindowFlags wflags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;

    // Active Users (top-left)
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(left_w, (float)_height - options_h - groups_h - spacing), ImGuiCond_Always);
    std::string title = "Usuários Ativos (" + std::to_string(_activeUsers ? _activeUsers->size() : 0) + ")";
    if (ImGui::Begin(title.c_str(), nullptr, wflags))
    {
        for (const auto &user : *_activeUsers)
        {
            ImGui::TextUnformatted(user.c_str());
            ImGui::SameLine();
            // If already in chat, show label instead of button
            if (_chats->find(user) != _chats->end())
            {
                ImGui::TextUnformatted(" (em conversa)");
            }
            else if (_pendingRequestsTo->find(user) != _pendingRequestsTo->end())
            {
                ImGui::TextUnformatted(" (solicitação enviada)");
            }
            else if (_pendingRequestsFrom->find(user) != _pendingRequestsFrom->end())
            {
                if (ImGui::Button("Aceitar"))
                {
                    // Accept chat request
                    _onChatRequestAccept(user.c_str());
                }
                ImGui::SameLine();
                if (ImGui::Button("Recusar"))
                {
                    // Decline chat request
                    _onChatRequestDecline(user.c_str());
                }
            }
            else if (ImGui::Button(("Chat##" + user).c_str()))
            {
                // Open chat with user
                _onChatRequestClick(user.c_str());
            }

            if(_groupRequestFrom->find(user) != _groupRequestFrom->end())
            {
                ImGui::SameLine();
                if(ImGui::Button("Aceitar##grupo"))
                {
                    // Accept group request
                    auto groupit = _groupRequestFrom->find(user);
                    _onGroupRequestAccept(groupit->second.c_str());
                }
                ImGui::SameLine();
                if(ImGui::Button("Recusar##grupo"))
                {
                    // Decline group request
                }
            }
        }

        if (_activeUsers->size() == 0)
            ImGui::TextUnformatted("Nenhum usuário ativo.");

        ImGui::End();
    }

    // Active Groups (middle-left)
    ImGui::SetNextWindowPos(ImVec2(0.f, (float)_height - options_h - groups_h - spacing * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(left_w, groups_h), ImGuiCond_Always);
    if (ImGui::Begin("Grupos Ativos", nullptr, wflags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::InputText("##groupname", _groupName, sizeof(_groupName));
        ImGui::SameLine();
        if (ImGui::Button("Criar Grupo"))
        {
            // Create group logic (not implemented)
            if (_onCreateGroupClick && _groupName[0] != '\0')
            {
                _onCreateGroupClick(_groupName);
                _groupName[0] = '\0';
            }
        }

        ImGui::NewLine();
        ImGui::TextUnformatted("Grupos disponíveis:");
        ImGui::Separator();

        ImGui::BeginChild("GroupList", ImVec2(0, groups_h - 60), false);

        int groupCount = 0;
        for (auto &chatPair : *_allGroups)
        {
            ImGui::TextUnformatted(chatPair.first.c_str());
            ImGui::SameLine();
            ImGui::TextUnformatted("- dono: ");
            ImGui::SameLine();
            ImGui::TextUnformatted(chatPair.second.owner.c_str());

            if (chatPair.second.owner != _username)
            {
                ImGui::SameLine();
                if(chatPair.second.isOpen)
                {
                    ImGui::TextUnformatted(" (em conversa)");    
                }
                else
                {
                    if (ImGui::Button(("Entrar##" + chatPair.second.groupName).c_str()))
                    {
                        // Open group chat
                        _onGroupRequestClick(chatPair.second.groupName.c_str());
                    }
                }
            }

            groupCount++;
        }

        if (groupCount == 0)
            ImGui::TextUnformatted("Nenhum grupo ativo.");

        ImGui::EndChild();

        ImGui::End();
    }

    // Options (bottom-left)
    ImGui::SetNextWindowPos(ImVec2(0.f, (float)_height - options_h), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(left_w, options_h), ImGuiCond_Always);
    if (ImGui::Begin("Opções", nullptr, wflags))
    {
        if (ImGui::Button("Sair"))
        {
            if (_onCloseButtonClick)
                _onCloseButtonClick();
        }
        ImGui::End();
    }

    // Chats (right side fills remaining)
    ImGui::SetNextWindowPos(ImVec2(left_w + spacing, 0.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)_width - (left_w + spacing), (float)_height), ImGuiCond_Always);
    if (ImGui::Begin("Chats", nullptr, wflags))
    {
        if (_chats && ImGui::BeginTabBar("ChatTabs"))
        {
            for (auto &chatPair : *_groupChats)
            {
                const std::string &chatId = chatPair.first;
                std::queue<Message> &messages = chatPair.second.messages;

                if (!chatPair.second.isOpen)
                {
                    _groupChats->erase(chatId);
                    break;
                }

                if (ImGui::BeginTabItem(chatId.c_str(), chatId == "Marcohub" ? nullptr : &chatPair.second.isOpen))
                {
                    // Messages list area
                    ImGui::BeginChild("Messages", ImVec2(0, -34), true);
                    std::queue<Message> temp = messages; // copy for iteration
                    while (!temp.empty())
                    {
                        const Message &msg = temp.front();

                        // Format timestamp to HH:MM
                        std::time_t timestamp = std::chrono::system_clock::to_time_t(msg.timestamp);
                        std::tm *tm_info = std::localtime(&timestamp);
                        timestamp = tm_info->tm_hour * 100 + tm_info->tm_min;
                        ImGui::Text("[%02d:%02d] %s: %s", tm_info->tm_hour, tm_info->tm_min, msg.sender, msg.content);
                        temp.pop();
                    }
                    ImGui::EndChild();

                    // Input area
                    ImGui::Text("Mensagem:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(-80);
                    bool enter = ImGui::InputText("##message", chatPair.second.draft, sizeof(chatPair.second.draft), ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::SameLine();
                    if (ImGui::Button("Enviar") || enter)
                    {
                        if (chatPair.second.draft[0] != '\0')
                        {
                            _onSendMessage(chatId.c_str(), chatPair.second.draft, MessageType::GROUP_CHAT);
                            chatPair.second.draft[0] = '\0';
                        }
                    }

                    ImGui::EndTabItem();
                }
            }

            for (auto &chatPair : *_chats)
            {
                const std::string &chatId = chatPair.first;
                std::queue<Message> &messages = chatPair.second.messages;

                if (!chatPair.second.isOpen)
                {
                    _chats->erase(chatId);
                    break;
                }

                if (ImGui::BeginTabItem(chatId.c_str(), &chatPair.second.isOpen))
                {
                    // Messages list area
                    ImGui::BeginChild("Messages", ImVec2(0, -34), true);
                    std::queue<Message> temp = messages; // copy for iteration
                    while (!temp.empty())
                    {
                        const Message &msg = temp.front();

                        // Format timestamp to HH:MM
                        std::time_t timestamp = std::chrono::system_clock::to_time_t(msg.timestamp);
                        std::tm *tm_info = std::localtime(&timestamp);
                        timestamp = tm_info->tm_hour * 100 + tm_info->tm_min;
                        ImGui::Text("[%02d:%02d] %s: %s", tm_info->tm_hour, tm_info->tm_min, msg.sender, msg.content);
                        temp.pop();
                    }
                    ImGui::EndChild();

                    // Input area
                    ImGui::Text("Mensagem:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(-80);
                    bool enter = ImGui::InputText("##message", chatPair.second.draft, sizeof(chatPair.second.draft), ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::SameLine();
                    if (ImGui::Button("Enviar") || enter)
                    {
                        if (chatPair.second.draft[0] != '\0')
                        {
                            _onSendMessage(chatId.c_str(), chatPair.second.draft, MessageType::PRIVATE_CHAT);
                            chatPair.second.draft[0] = '\0';
                        }
                    }

                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
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

void Window::setUsername(const char *username)
{
    strcpy(_username, username);
}
