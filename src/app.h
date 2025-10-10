#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "client.h"
#include "window.h"

#define BROKER_ADDRESS "tcp://localhost:1883"

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
    void onStatusMessage(Message* message);
    void onChatMessage(Message* message);

private:
    Window* _window;
    Client* _client;

    bool _running;

    std::vector<std::string> _activeUsers;
    // std::unordered_map<std::string, std::vector<Message>> _userMessageBuffers;
    // std::unordered_map<std::string, std::vector<Message>> _chats; // Key: username, Value: list of messages

    static App* _instance;
};