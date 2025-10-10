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
    STATUS,
    CHAT
};
struct Message
{
    MessageType type;
    const char* sender;
    const char* content;
    std::chrono::system_clock::time_point timestamp;
};

class Client
{
public:
    Client(const char* address, const char* clientId);
    ~Client();

    inline void setOnStatusMessageCallback(const std::function<void(Message*)>& callback) { _onStatusMessage = callback; }

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

    static Client* _instance;
};