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
    _window->setActiveUsersVector(&_activeUsers);

    _client = nullptr; // Client will be initialized after login
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
        statusMessage.sender = _client->getClientId();
        statusMessage.content = "offline";
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
        _client->connect();
        _client->subscribe("dev/marconautas/user/status/+"); // Subscribe to user status updates
        _client->subscribe("dev/marconautas/marcohub");      // Subscribe to global chat

        // Publish online status
        std::string statusTopic = "dev/marconautas/user/status/" + std::string(username);

        Message statusMessage;
        statusMessage.type = MessageType::STATUS;
        statusMessage.sender = username;
        statusMessage.content = "online";
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

void App::onStatusMessage(Message *message)
{
    if (message->content == "online")
    {
        // Add user to active users list if not already present
        if (std::find(_activeUsers.begin(), _activeUsers.end(), message->sender) == _activeUsers.end())
        {
            _activeUsers.push_back(message->sender);
        }
    }
    else if (message->content == "offline")
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

void App::onChatMessage(Message *message)
{
}
