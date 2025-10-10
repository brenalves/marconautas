#include "app.h"

App *App::s_instance = nullptr;

App::App()
    : m_running(true), m_isLogged(false), m_client(nullptr), m_username("")
{
    if (s_instance)
    {
        throw std::runtime_error("Apenas uma instancia do App pode existir!!");
    }

    s_instance = this;

    if (!glfwInit())
    {
        throw std::runtime_error("Falha ao iniciar a biblioteca GLFW :O");
    }

    glfwDefaultWindowHints();

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    m_window = glfwCreateWindow(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, "Marconautas", nullptr, nullptr);
    if (!m_window)
    {
        throw std::runtime_error("Falha ao criar janela :O ");
    }

    glfwMakeContextCurrent(m_window);

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window)
                               { App::getInstance()->close(); });

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // ini file at root folder
    io.IniFilename = "../ui.ini";

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

App::~App()
{
    if (m_isLogged)
    {
        std::string onlineTopic = "dev/marconautas/user/status/" + std::string(m_username);
        std::string offlineMessage = "offline:" + std::string(m_username);
        MQTTPublish(onlineTopic.c_str(), offlineMessage.c_str());
    }

    // Libera recursos alocados pelo MQTTClient
    std::cout << "Destruindo cliente MQTT" << std::endl;
    ;
    MQTTClient_disconnect(m_client, 1000);
    MQTTClient_destroy(&m_client);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
}

void App::run()
{
    while (m_running)
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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

        if (!m_isLogged)
        {
            ImGui::Begin("Login");
            ImGui::Text("Bem vindo ao Marcomundo");
            ImGui::Text("Your username:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);
            ImGui::InputText("##inputLogin", m_username, sizeof(m_username));
            if (ImGui::Button("Entrar"))
            {
                int rc;

                // Initialize MQTT
                if ((rc = MQTTClient_create(&m_client, MQTT_URL, m_username, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
                {
                    throw std::runtime_error("Falha ao criar cliente MQTT, abortando... :( | Code: " + std::to_string(rc));
                }

                std::cout << "MQTTClient criado com sucesso" << std::endl;

                // Initialize setCallbacks
                if ((rc = MQTTClient_setCallbacks(m_client, NULL, NULL, onMessageArrived, NULL)) != MQTTCLIENT_SUCCESS)
                {
                    throw std::runtime_error("Falha ao setar callbacks, abortando... :( | Code: " + std::to_string(rc));
                }

                std::cout << "Callbacks inicializadas com sucesso" << std::endl;

                // Set opções de will
                MQTTClient_willOptions will_option = MQTTClient_willOptions_initializer;
                std::string offlineMessage = "offline:" + std::string(m_username);
                will_option.message = offlineMessage.c_str();
                will_option.topicName = "marconautas/a";
                will_option.qos = 1;
                will_option.retained = 1;
                std::cout << "Configurações de deconexão repentina configurada" << std::endl;

                // Initialize opções de conexão
                MQTTClient_connectOptions conn_options = MQTTClient_connectOptions_initializer;
                // conn_options.will = &will_option;
                conn_options.keepAliveInterval = 20;
                conn_options.cleansession = 1;
                std::cout << "Configuracoes de cliente inicializadas" << std::endl;

                // Conecta cliente
                if ((rc = MQTTClient_connect(m_client, &conn_options)) != MQTTCLIENT_SUCCESS)
                {
                    throw std::runtime_error("Falha ao conectar ao servidor (" + std::string(MQTT_URL) + ") | Code: " + std::to_string(rc));
                }

                std::cout << "Cliente conectado, bem vindo(a) " << m_username << std::endl;
                glfwSetWindowTitle(m_window, ("Marconautas - " + std::string(m_username)).c_str());

                // Inscrição no tópico MarcoHub e tópico pessoal
                MQTTSubscribe("dev/marconautas/marcohub");
                std::string ownTopic = "dev/marconautas/user/" + std::string(m_username);
                MQTTSubscribe(ownTopic.c_str());
                MQTTSubscribe("dev/marconautas/user/status/+");

                std::cout << "Inscrições realizadas com sucesso" << std::endl;

                std::string onlineTopic = "dev/marconautas/user/status/" + std::string(m_username);
                std::string onlineMessage = "online:" + std::string(m_username);
                MQTTPublish(onlineTopic.c_str(), onlineMessage.c_str());

                m_isLogged = true;
            }
            ImGui::End();
        }
        else
        {
            ImGui::Begin("Usuários ativos");
            ImGui::End();

            if (ImGui::Begin("Options"))
            {

                if (ImGui::Button("Disconnect"))
                {
                    close();
                }

                ImGui::End();
            }

            if (ImGui::Begin("Chats"))
            {
                if (ImGui::BeginTabBar("Chats a"))
                {

                    if (ImGui::BeginTabItem("Marcohub"))
                    {
                        ImGui::Text("Conteudo aba marcohub");

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("luanzinho"))
                    {
                        ImGui::Text("Conteudo aba luanzinho");

                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }

                ImGui::End();
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }
}

void App::MQTTSubscribe(const char *topic)
{
    int rc;

    if ((rc = MQTTClient_subscribe(m_client, topic, 1)) != MQTTCLIENT_SUCCESS)
    {
        throw std::runtime_error("Failed to subscribe to topic" + std::string(MQTT_URL) + " | Code: " + std::to_string(rc));
    }

    std::cout << "Inscrito no tópico " << topic << std::endl;
}

int App::onMessageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    std::string msgPayload(static_cast<char *>(message->payload), message->payloadlen);

    std::cout << "Mensagem recebida no tópico " << topicName << ": " << msgPayload << std::endl;

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return 1;
}

void App::MQTTPublish(const char *topic, const char *message)
{
    int rc;

    MQTTClient_message msg = MQTTClient_message_initializer;
    msg.payload = (void *)message;
    msg.payloadlen = (int)strlen(message);
    msg.qos = 1;
    msg.retained = 1;

    MQTTClient_deliveryToken token;

    if ((rc = MQTTClient_publishMessage(m_client, topic, &msg, &token)) != MQTTCLIENT_SUCCESS)
    {
        throw std::runtime_error("Failed to publish message at topic" + std::string(topic) + " | Code: " + std::to_string(rc));
    }

    std::cout << "Mensagem publicada no tópico " << topic << ": " << message << std::endl;
}