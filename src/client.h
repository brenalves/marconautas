#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <MQTTClient.h>

class Client
{
public:

    Client(char* username);
    ~Client();

private:

    MQTTClient mqtt_client;

    bool initializeClient();
};

