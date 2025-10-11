#include "app.h"

App *App::_instance = nullptr;

App::App()
{
    if (_instance)
    {
        throw std::runtime_error("Apenas uma instancia do App pode existir!!");
    }

    _instance = this;

    _window = new Window(800, 600, "Marconautas");
    _window->setOnCloseButtonClick(std::bind(&App::onCloseButtonClick, this));
    _window->setOnLoginButtonSubmit(std::bind(&App::onLoginButtonSubmit, this, std::placeholders::_1));
    _window->setOnChatRequestClick(std::bind(&App::onChatRequestClick, this, std::placeholders::_1));

    _window->setOnSendMessage(std::bind(&App::onSendMessage, this, std::placeholders::_1, std::placeholders::_2));
    _window->setActiveUsersVector(&_activeUsers);
    _window->setChatsMap(&_chats);

    _client = nullptr; // Client will be initialized after login

    _chats.insert({"Marcohub", Chat()}); // Global chat
}

App::~App()
{
    // Cleanup
    _instance = nullptr;

    if (_client)
    {
        // Publish offline status if client is connected
        std::string statusTopic = "dev/marconautas/user/status/" + _client->getClientId();

        Message statusMessage;
        statusMessage.type = MessageType::STATUS;
        strcpy(statusMessage.sender, _client->getClientId().c_str());
        strcpy(statusMessage.content, "offline");
        statusMessage.timestamp = std::chrono::system_clock::now();
        _client->publish(statusTopic.c_str(), statusMessage, true);
        
        _client->disconnect();
        delete _client;
    }

    if (_window)
        delete _window;
}

void App::run()
{
    _running = true;

    // Main loop
    while (_running)
    {
        _window->beginFrame();

        if (!_client)
            _window->showLogin(); // Show login screen first
        else
            _window->showMain(); // Then show main screen after login

        _window->endFrame();
    }
}

void App::onCloseButtonClick()
{
    _running = false;
}

void App::onLoginButtonSubmit(const char *username)
{
    try
    {
        if (_client)
            throw std::runtime_error("Client already initialized!");

        _client = new Client(BROKER_ADDRESS, username);
        _client->setOnStatusMessageCallback(std::bind(&App::onStatusMessage, this, std::placeholders::_1));
        _client->setOnChatMessageCallback(std::bind(&App::onChatMessage, this, std::placeholders::_1, std::placeholders::_2));
        _client->connect();
        _client->subscribe("dev/marconautas/user/status/+"); // Subscribe to user status updates
        _client->subscribe("dev/marconautas/marcohub");      // Subscribe to global chat
        std::string ownTopic = "dev/marconautas/user/" + std::string(username);
        _client->subscribe(ownTopic.c_str());               // Subscribe to own chat topic

        // Publish online status
        std::string statusTopic = "dev/marconautas/user/status/" + std::string(username);

        Message statusMessage;
        statusMessage.type = MessageType::STATUS;
        strcpy(statusMessage.sender, username);
        strcpy(statusMessage.content, "online");
        statusMessage.timestamp = std::chrono::system_clock::now();
        _client->publish(statusTopic.c_str(), statusMessage, true);

        _window->setTitle((std::string("Marconautas - ") + username).c_str());
    }
    catch (const std::exception &e)
    {
        std::cout << "Erro ao conectar: " << e.what() << std::endl;

        delete _client;
        _running = false;
        return;
    }
}

void App::onChatRequestClick(const char *target)
{
    if(_chats.find(target) == _chats.end())
    {
        _chats.insert({target, Chat()});

        std::string topic = "dev/marconautas/user/" + std::string(target);

        Message welcomeMsg;
        welcomeMsg.type = MessageType::CHAT;
        strcpy(welcomeMsg.sender, _client->getClientId().c_str());
        strcpy(welcomeMsg.content, "Oi! Vamos conversar?");
        welcomeMsg.timestamp = std::chrono::system_clock::now();
        _client->publish(topic.c_str(), welcomeMsg);

        _chats[target].messages.push(welcomeMsg);
    }
}

void App::onSendMessage(const char *topic, const char *message)
{
    std::cout << "Sending message to topic " << topic << ": " << message << std::endl;
    
    Message chatMessage;
    chatMessage.type = MessageType::CHAT;
    strcpy(chatMessage.sender, _client->getClientId().c_str());
    strcpy(chatMessage.content, message);
    chatMessage.timestamp = std::chrono::system_clock::now();

    std::string fullTopic;
    if(strcmp(topic, "Marcohub") == 0)
        fullTopic = "dev/marconautas/marcohub";
    else
        fullTopic = "dev/marconautas/user/" + std::string(topic);

    _client->publish(fullTopic.c_str(), chatMessage);

    _chats[topic].messages.push(chatMessage);
}

void App::onStatusMessage(Message *message)
{
    if (strcmp(message->content, "online") == 0)
    {
        // Add user to active users list if not already present
        if (std::find(_activeUsers.begin(), _activeUsers.end(), message->sender) == _activeUsers.end())
        {
            _activeUsers.push_back(message->sender);
        }
    }
    else if (strcmp(message->content, "offline") == 0)
    {
        // Remove user from active users list
        auto it = std::find(_activeUsers.begin(), _activeUsers.end(), message->sender);
        if (it != _activeUsers.end())
        {
            _activeUsers.erase(it);
        }
    }

    std::cout << "User " << message->sender << " is " << message->content << std::endl;
}

void App::onChatMessage(const char* topic, Message *message)
{
    if(strcmp(topic, "dev/marconautas/marcohub") == 0)
    {
        // Global chat message
        _chats["Marcohub"].messages.push(*message);
        return;
    }

    if (_chats.find(message->sender) != _chats.end())
    {
        _chats[message->sender].messages.push(*message);
    }
    else
    {
        Chat newChat;
        newChat.draft[0] = '\0';
        newChat.messages.push(*message);

        // Verify if the topic indicates a group chat ("dev/marconautas/group/")
        if (strncmp(topic, "dev/marconautas/group/", 22) == 0)
        {
            newChat.isGroup = true;
        }

        _chats[message->sender] = newChat;
    }
}
