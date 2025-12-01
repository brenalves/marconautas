#pragma once

#include <MQTTClient.h>
#include <string>
#include <chrono>
#include <stdexcept>
#include <queue>
#include <iostream>
#include <functional>
#include <cstring>

#define QOS 1
#define TIMEOUT 10000L

enum MessageType
{
    STATUS = 3214,
    PRIVATE_CHAT,
    GROUP_CHAT,
    PRIVATE_REQUEST,
    GROUP_REQUEST,
    GROUP_CHAT_CREATION,
};
struct Message
{
    int type; // Use MessageType enum values
    char sender[64];
    char content[256];
    std::chrono::time_point<std::chrono::system_clock> timestamp; // Timestamp
};

class Client
{
public:
    Client(const char* address, const char* clientId);
    ~Client();

    inline void setOnStatusMessageCallback(const std::function<void(Message*)>& callback) { _onStatusMessage = callback; }
    inline void setOnChatMessageCallback(const std::function<void(const char*, Message*)>& callback) { _onChatMessage = callback; }
    inline void setOnRequestMessageCallback(const std::function<void(const char*, Message*)>& callback) { _onRequestMessage = callback; }
    inline void setOnChatCreationMessageCallback(const std::function<void(const char*, Message*)>& callback) { _onChatCreationMessage = callback; }

    void connect();
    void disconnect();
    void subscribe(const char* topic);
    void publish(const char* topic, Message& message, bool retained = false);

    inline std::string& getClientId() { return _clientId; }

private:
    static int onMessageArrived(void* context, char* topicName, int topicLen, MQTTClient_message* message);

private:
    MQTTClient _client;
    int _returnCode;

    std::string _address;
    std::string _clientId;

    std::function<void(Message*)> _onStatusMessage;
    std::function<void(const char* , Message*)> _onChatMessage;
    std::function<void(const char*, Message*)> _onRequestMessage;
    std::function<void(const char*, Message*)> _onChatCreationMessage;

    static Client* _instance;
};