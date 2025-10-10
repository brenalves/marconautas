#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "client.h"
#include "window.h"

#define BROKER_ADDRESS "tcp://localhost:1883"
#define TOPICS_PREFIX "devdev/marconautas/"

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

private:
    Window* _window;
    Client* _client;

    bool _running;

    std::vector<std::string> _activeUsers;

    static App* _instance;
};