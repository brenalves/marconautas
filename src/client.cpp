#include "client.h"

#include <iostream>
#include <MQTTClient.h>
#include <chrono>
#include <string>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define MQTT_URL "tcp://test.mosquitto.org:1883"

Client::Client(const char* username)
    : username(username),    
      isLogged(false),       
      mqtt_client(nullptr)   
{
    if (!initializeClient()) {
        std::cerr << "Falha ao inicializar o cliente MQTT." << std::endl;
    }
}


Client::~Client()
{
    // Libera recursos alocados pelo MQTTClient
    MQTTClient_disconnect(mqtt_client, 1000);
    MQTTClient_destroy(&mqtt_client);
}

bool Client::initializeClient()
{
    
}