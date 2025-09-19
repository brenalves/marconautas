#include <iostream>
#include <MQTTClient.h>
#include <chrono>
#include <string>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define MQTT_URL "tcp://test.mosquitto.org:1883"

struct MQTTMessage
{
    char sender[64];
    char content[256];
    int timestamp;
};

bool isLogged = false;
char username[64] = "";
MQTTMessage message;
MQTTClient client;

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message);

int main()
{
    std::cout << "Hello World" << std::endl;

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW, aborting..." << std::endl;
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(800, 600, "Marconautas - Login", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create window, aborting..." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 22.0f;

    io.Fonts->AddFontDefault(&fontConfig);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!isLogged)
        {
            // Get the current screen size (or window size if in a container)
            ImVec2 screenSize = ImGui::GetIO().DisplaySize;

            // Set the next window to occupy the full screen
            ImGui::SetNextWindowSize(screenSize);  // Set window size to screen size
            ImGui::SetNextWindowPos(ImVec2(0, 0)); // Set window position to the top-left corner

            ImGui::Begin("User Form", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text("Bem vindo ao Marconautas");
            ImGui::Text("Username: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200.0f);
            ImGui::InputText("##Username", username, IM_ARRAYSIZE(username));
            ImGui::SameLine();
            if (ImGui::Button("Entrar", ImVec2(100, 0)))
            {
                int rc;
                if ((rc = MQTTClient_create(&client, MQTT_URL, username, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
                {
                    std::cout << "Failed to create MQTTClient, error code: " << rc << std::endl;
                    glfwSetWindowShouldClose(window, true);
                }

                if ((rc = MQTTClient_setCallbacks(client, NULL, NULL, onMessageArrived, NULL)) != MQTTCLIENT_SUCCESS)
                {
                    std::cout << "Failed to set MQTT callbacks, error code: " << rc << std::endl;
                    glfwSetWindowShouldClose(window, true);
                }

                MQTTClient_connectOptions conn_options = MQTTClient_connectOptions_initializer;
                conn_options.keepAliveInterval = 20;
                conn_options.cleansession = 1;
                if ((rc = MQTTClient_connect(client, &conn_options)) != MQTTCLIENT_SUCCESS)
                {
                    std::cout << "Failed to connect to broker " << MQTT_URL << ", error code: " << rc << std::endl;
                    glfwSetWindowShouldClose(window, true);
                }

                if((rc = MQTTClient_subscribe(client, "marconautas/marcohub", 1)) != MQTTCLIENT_SUCCESS)
                {
                    std::cout << "Failed to subscribe to topic, error code:" << rc << std::endl;
                    glfwSetWindowShouldClose(window, true);
                }

                // std::string ownTopic = "marconautas/user/" + std::string(username);
                // if((rc = MQTTClient_subscribe(client, ownTopic.c_str(), 1)) != MQTTCLIENT_SUCCESS)
                // {
                //     std::cout << "Failed to subscribe to topic, error code:" << rc << std::endl;
                //     glfwSetWindowShouldClose(window, true);
                // }

                isLogged = true;

                std::cout << "Logged as " << username << std::endl;

                glfwSetWindowTitle(window, "Marcohub");
                glfwSetWindowSize(window, 1360, 768);

                strcpy(message.sender, username);
            }
            ImGui::End();
        }
        else
        {
            // Get the current screen size (or window size if in a container)
            ImVec2 screenSize = ImGui::GetIO().DisplaySize;

            // Set the next window to occupy the full screen
            ImGui::SetNextWindowSize(screenSize);  // Set window size to screen size
            ImGui::SetNextWindowPos(ImVec2(0, 0)); // Set window position to the top-left corner

            ImGui::Begin("MarcoHub", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text("Type message: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200.0f);
            ImGui::InputText("##Message", message.content, IM_ARRAYSIZE(message.content));
            ImGui::SameLine();
            if (ImGui::Button("Enviar", ImVec2(100, 0)))
            {
                int rc;

                auto now = std::chrono::system_clock::now();
                message.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

                MQTTClient_message mqttMsg = MQTTClient_message_initializer;
                mqttMsg.payload = &message;
                mqttMsg.payloadlen = sizeof(message);
                mqttMsg.qos = 1;
                mqttMsg.retained = 0;

                if ((rc = MQTTClient_publishMessage(client, "marconautas/marcohub", &mqttMsg, NULL)) != MQTTCLIENT_SUCCESS)
                {
                    std::cout << "Failed to send message" << std::endl;
                }
                else
                {
                    memset(message.content, 0, sizeof(message));
                }
            }

            ImGui::End();
        }

        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    MQTTClient_destroy(&client);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    MQTTMessage received;
    memcpy(&received, message->payload, sizeof(MQTTMessage));
    std::cout << "Sender: " << received.sender << "\n" << received.content << "\nTimestamp: " << received.timestamp << std::endl;
    return 1;
}
