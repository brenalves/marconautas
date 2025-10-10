#pragma once

#include <memory>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <MQTTClient.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>

#define WINDOW_DEFAULT_WIDTH 800
#define WINDOW_DEFAULT_HEIGHT 600
#define MQTT_URL "tcp://test.mosquitto.org:1883"

struct Message
{
    const char* sender;
    const char* content;
    std::chrono::system_clock::time_point timestamp;
};

class App
{
public:
    App();
    ~App();

    void run();

    inline void close() { m_running = false; }
    
    inline static App* getInstance() { return s_instance; }
    
private:
    void MQTTSubscribe(const char* topic);
    void MQTTPublish(const char* topic, Message& message, bool retained);

    static int onMessageArrived(void* context, char* topicName, int topicLen, MQTTClient_message* message);

private:
    GLFWwindow* m_window;

    MQTTClient m_client;

    char m_username[25];

    bool m_running;
    bool m_isLogged;

    std::vector<std::string> m_activeUsers;

    static App* s_instance;
};