#include "client.h"

Client *Client::_instance = nullptr;

Client::Client(const char *address, const char *clientId)
    : _address(address), _clientId(clientId)
{
    if (_instance)
    {
        throw std::runtime_error("Only one instance of Client can exist!!");
    }

    _instance = this;

    if ((_returnCode = MQTTClient_create(&_client, address, clientId, MQTTCLIENT_PERSISTENCE_NONE, nullptr)) != MQTTCLIENT_SUCCESS)
    {
        throw std::runtime_error("Failed to create MQTT client, return code: " + std::to_string(_returnCode));
    }

    if ((_returnCode = MQTTClient_setCallbacks(_client, nullptr, nullptr, Client::onMessageArrived, nullptr)) != MQTTCLIENT_SUCCESS)
    {
        throw std::runtime_error("Failed to set MQTT callbacks, return code: " + std::to_string(_returnCode));
    }

    std::cout << "MQTT client created successfully." << std::endl;
}

Client::~Client()
{
    MQTTClient_destroy(&_client);
}

void Client::connect()
{
    MQTTClient_connectOptions connOpts = MQTTClient_connectOptions_initializer;
    connOpts.keepAliveInterval = 20;
    connOpts.cleansession = 1;

    if ((_returnCode = MQTTClient_connect(_client, &connOpts)) != MQTTCLIENT_SUCCESS)
    {
        throw std::runtime_error("Failed to connect to MQTT broker, return code: " + std::to_string(_returnCode));
    }

    std::cout << "Connected to MQTT broker successfully." << std::endl;
}

void Client::disconnect()
{
    if ((_returnCode = MQTTClient_disconnect(_client, TIMEOUT)) != MQTTCLIENT_SUCCESS)
    {
        throw std::runtime_error("Failed to disconnect from MQTT broker, return code: " + std::to_string(_returnCode));
    }

    std::cout << "Disconnected from MQTT broker successfully." << std::endl;
}

void Client::subscribe(const char *topic)
{
    if ((_returnCode = MQTTClient_subscribe(_client, topic, QOS)) != MQTTCLIENT_SUCCESS)
    {
        throw std::runtime_error("Failed to subscribe to topic " + std::string(topic) + ", return code: " + std::to_string(_returnCode));
    }

    std::cout << "Subscribed to topic " << topic << " successfully." << std::endl;
}

void Client::publish(const char *topic, Message &message, bool retained)
{
    MQTTClient_message pubMsg = MQTTClient_message_initializer;
    pubMsg.payload = (void *)&message;
    pubMsg.payloadlen = sizeof(message);
    pubMsg.qos = QOS;
    pubMsg.retained = retained;

    if ((_returnCode = MQTTClient_publishMessage(_client, topic, &pubMsg, nullptr)) != MQTTCLIENT_SUCCESS)
    {
        throw std::runtime_error("Failed to publish message to topic " + std::string(topic) + ", return code: " + std::to_string(_returnCode));
    }

    std::cout << "Published message to topic " << topic << " successfully." << std::endl;
}

int Client::onMessageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    Message *msg = (Message *)message->payload;

    if (message->payloadlen != sizeof(Message))
    {
        std::cout << "Received message with invalid size from topic " << topicName << std::endl;
        MQTTClient_freeMessage(&message);
        MQTTClient_free(topicName);
        return 1;
    }

    if (strcmp(msg->sender, _instance->getClientId().c_str()) == 0)
    {
        // Ignore messages sent by ourselves
        MQTTClient_freeMessage(&message);
        MQTTClient_free(topicName);
        return 1;
    }

    // Check if the message is a status message
    if (msg->type == MessageType::STATUS)
    {
        _instance->_onStatusMessage(msg);
    }
    else if (msg->type == MessageType::CHAT)
    {
        _instance->_onChatMessage(topicName, msg);
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return 1;
}
