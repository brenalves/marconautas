#pragma once

#include <MQTTClient.h>
#include <string>
#include <chrono>
#include <stdexcept>
#include <queue>
#include <iostream>
#include <functional>

#define QOS 1
#define TIMEOUT 10000L

enum class MessageType
{
    STATUS = 3214,
    CHAT
};
struct Message
{
    MessageType type;
    std::string sender;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
};

class Client
{
public:
    Client(const char* address, const char* clientId);
    ~Client();

    inline void setOnStatusMessageCallback(const std::function<void(Message*)>& callback) { _onStatusMessage = callback; }
    inline void setOnChatMessageCallback(const std::function<void(Message*)>& callback) { _onChatMessage = callback; }

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
    std::function<void(Message*)> _onChatMessage;

    static Client* _instance;
};