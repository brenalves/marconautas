#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <nlohmann/json.hpp>
#include <fstream>

#include "client.h"
#include "window.h"

#define BROKER_ADDRESS "tcp://test.mosquitto.org:1883"
#define JSON_PATH "../db.json"

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
    void onChatRequestAccept(const char* target);
    void onChatRequestDecline(const char* target);
    void onSendMessage(const char* topic, const char* message);

    void onStatusMessage(Message* message);
    void onChatMessage(const char* topic, Message* message);
    void onRequestMessage(const char* topic, Message* message);

private:
    Window* _window;
    Client* _client;

    nlohmann::json _db;

    bool _running;
    bool _ownStatusMessageSent;

    std::vector<std::string> _activeUsers;
    std::unordered_map<std::string, Chat> _chats;
    std::unordered_map<std::string, bool> _pendingRequestsFrom;
    std::unordered_map<std::string, bool> _pendingRequestsTo;

    static App* _instance;
};