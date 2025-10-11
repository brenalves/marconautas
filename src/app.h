#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <queue>

#include "client.h"
#include "window.h"

#define BROKER_ADDRESS "tcp://test.mosquitto.org:1883"

class App
{
public:
    App();
    ~App();

    void run();

    inline static App* getInstance() { return _instance; }

private:
    void onCloseButtonClick();
    void onLoginButtonSubmit(const char* username);
    void onChatRequestClick(const char* target);
    void onSendMessage(const char* topic, const char* message);

    void onStatusMessage(Message* message);
    void onChatMessage(const char* topic, Message* message);

private:
    Window* _window;
    Client* _client;

    bool _running;

    std::vector<std::string> _activeUsers;
    std::unordered_map<std::string, Chat> _chats;

    static App* _instance;
};