#include "app.h"

using json = nlohmann::json;

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
    _window->setOnChatRequestAccept(std::bind(&App::onChatRequestAccept, this, std::placeholders::_1));
    _window->setOnChatRequestDecline(std::bind(&App::onChatRequestDecline, this, std::placeholders::_1));
    _window->setOnCreateGroupClick(std::bind(&App::onCreateGroupClick, this, std::placeholders::_1));
    _window->setOnSendMessage(std::bind(&App::onSendMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    
    _window->setActiveUsersVector(&_activeUsers);
    _window->setChatsMap(&_chats);
    _window->setGroupChatsMap(&_groupChats);
    _window->setPendingRequestsFromMap(&_pendingRequestsFrom);
    _window->setPendingRequestsToMap(&_pendingRequestsTo);
    _window->setGroupRequestsFromMap(&_groupRequestFrom);
    _window->setGroupRequestsToMap(&_groupRequestTo);
    _window->setAllGroupsMap(&_allGroups);

    _client = nullptr; // Client will be initialized after login
    _ownStatusMessageSent = false;

    _groupChats.insert({"Marcohub", GroupChat()}); // Global chat

    // Load or create JSON database
    if (!std::filesystem::exists(JSON_PATH))
    {
        std::cout << "Arquivo não existe. Criando JSON vazio...\n";

        _db = json::object();

        // Cria o arquivo e escreve o JSON vazio
        std::ofstream dbFile(JSON_PATH);
        dbFile << std::setw(4) << _db << std::endl;
        dbFile.close();
    }
    else
    {
        std::ifstream dbFile(JSON_PATH);

        // Verifica se o arquivo está vazio
        std::cout << "Arquivo existe. Carregando...\n";
        dbFile >> _db;

        dbFile.close();
    }
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
        _client->setOnRequestMessageCallback(std::bind(&App::onRequestMessage, this, std::placeholders::_1, std::placeholders::_2));
        _client->setOnChatCreationMessageCallback(std::bind(&App::onChatCreationMessage, this, std::placeholders::_1, std::placeholders::_2));
        _client->connect();
        _client->subscribe("dev/marconautas/marcohub"); // Subscribe to global chat
        std::string ownTopic = "dev/marconautas/user/" + std::string(username);
        _client->subscribe("dev/marconautas/user/status/+"); // Subscribe to user status updates
        _client->subscribe(ownTopic.c_str());                // Subscribe to own chat topic
        std::string controlTopic = "dev/marconautas/user/" + std::string(username) + "/control";
        _client->subscribe(controlTopic.c_str());      // Subscribe to control messages
        _client->subscribe("dev/marconautas/group/+"); // Subscribe to group chats

        // Publish online status
        std::string statusTopic = "dev/marconautas/user/status/" + std::string(username);

        Message statusMessage;
        statusMessage.type = MessageType::STATUS;
        strcpy(statusMessage.sender, username);
        strcpy(statusMessage.content, "online");
        statusMessage.timestamp = std::chrono::system_clock::now();
        _client->publish(statusTopic.c_str(), statusMessage, true);
        _ownStatusMessageSent = true;

        _window->setTitle((std::string("Marconautas - ") + username).c_str());
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "Erro ao conectar: " << e.what() << std::endl;

        delete _client;
        _running = false;
        return;
    }
    catch (const std::domain_error &e)
    {
        std::cout << "Erro de dominio ao conectar: " << e.what() << std::endl;

        _window->setUsername("");
        return;
    }
}

void App::onChatRequestClick(const char *target)
{
    if (_chats.find(target) == _chats.end())
    {
        std::string topic = "dev/marconautas/user/" + std::string(target) + "/control";

        Message controlMsg;
        controlMsg.type = MessageType::PRIVATE_REQUEST;
        strcpy(controlMsg.sender, _client->getClientId().c_str());
        strcpy(controlMsg.content, "chat_request");
        controlMsg.timestamp = std::chrono::system_clock::now();
        _client->publish(topic.c_str(), controlMsg);
    }
}

void App::onChatRequestAccept(const char *target)
{
    std::cout << "Chat request accepted from " << target << std::endl;
    _chats[target] = PrivateChat(); // Create new chat
    _pendingRequestsFrom.erase(target);

    // Send confirmation message
    std::string topic = "dev/marconautas/user/" + std::string(target) + "/control";
    Message controlMsg;
    controlMsg.type = MessageType::PRIVATE_REQUEST;
    strcpy(controlMsg.sender, _client->getClientId().c_str());
    strcpy(controlMsg.content, "chat_accepted");
    controlMsg.timestamp = std::chrono::system_clock::now();
    _client->publish(topic.c_str(), controlMsg);
}

void App::onChatRequestDecline(const char *target)
{
    std::cout << "Chat request declined from " << target << std::endl;
    _pendingRequestsFrom.erase(target);
}

void App::onSendMessage(const char *topic, const char *message, int type)
{
    std::cout << "Sending message to topic " << topic << ": " << message << std::endl;

    Message chatMessage;
    chatMessage.type = static_cast<MessageType>(type);
    strcpy(chatMessage.sender, _client->getClientId().c_str());
    strcpy(chatMessage.content, message);
    chatMessage.timestamp = std::chrono::system_clock::now();

    std::string fullTopic;
    if (strcmp(topic, "Marcohub") == 0)
        fullTopic = "dev/marconautas/marcohub";
    else
        fullTopic = "dev/marconautas/user/" + std::string(topic);

    _client->publish(fullTopic.c_str(), chatMessage);

    if (type == MessageType::GROUP_CHAT)
        _groupChats[topic].messages.push(chatMessage);
    else if (type == MessageType::PRIVATE_CHAT)
        _chats[topic].messages.push(chatMessage);
}

void App::onCreateGroupClick(const char *groupName)
{
    std::cout << "Creating group chat: " << groupName << std::endl;

    // Create new chat
    GroupChat newChat;
    newChat.draft[0] = '\0';
    newChat.groupName = groupName;
    newChat.owner = _client->getClientId();
    _groupChats[groupName] = newChat;

    Message groupMessage;
    groupMessage.type = MessageType::GROUP_CHAT_CREATION;
    strcpy(groupMessage.sender, _client->getClientId().c_str());
    strcpy(groupMessage.content, "Group chat created");
    groupMessage.timestamp = std::chrono::system_clock::now();
    _client->publish(("dev/marconautas/group/" + std::string(groupName)).c_str(), groupMessage, true);

    _client->subscribe(("dev/marconautas/group/" + std::string(groupName)).c_str());

    _groupChats[groupName].messages.push(groupMessage);
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

void App::onChatMessage(const char *topic, Message *message)
{
    std::time_t timestamp = std::chrono::system_clock::to_time_t(message->timestamp);
    std::tm *tm_info = std::localtime(&timestamp);
    char timeStr[6];
    std::snprintf(timeStr, sizeof(timeStr), "%02d:%02d", tm_info->tm_hour, tm_info->tm_min);

    _db[topic]["messages"].push_back(json{{"sender", std::string(message->sender)}, {"content", std::string(message->content)}, {"timestamp", timeStr}});
    std::cout << std::setw(4) << _db << std::endl;
    std::cout << std::string(message->sender) << std::endl;
    std::cout << std::string(message->content) << std::endl;
    std::cout << timeStr << std::endl;

    if (strcmp(topic, "dev/marconautas/marcohub") == 0)
    {
        // Global chat message
        _groupChats["Marcohub"].messages.push(*message);

        return;
    }

    MessageType msgType = static_cast<MessageType>(message->type);

    if (msgType == GROUP_CHAT)
    {
        std::string groupName = std::string(topic).substr(std::string("dev/marconautas/group/").length());
        if (_groupChats.find(groupName) != _groupChats.end())
        {
            _groupChats[groupName].messages.push(*message);
        }
        else
        {
            GroupChat newGroupChat;
            newGroupChat.draft[0] = '\0';
            newGroupChat.groupName = groupName;
            newGroupChat.owner = _allGroups[groupName].owner;
            newGroupChat.messages.push(*message);

            _groupChats[groupName] = newGroupChat;
        }
    }
    else if (msgType == PRIVATE_CHAT)
    {
        if (_chats.find(message->sender) != _chats.end())
        {
            _chats[message->sender].messages.push(*message);
        }
        else
        {
            PrivateChat newChat;
            newChat.draft[0] = '\0';
            newChat.messages.push(*message);

            _chats[message->sender] = newChat;
        }
    }
}

void App::onRequestMessage(const char *topic, Message *message)
{
    if (strcmp(message->content, "chat_accepted") == 0)
    {
        std::cout << "Chat request accepted by " << message->sender << std::endl;
        _chats[message->sender] = PrivateChat(); // Create new chat
        _pendingRequestsTo.erase(message->sender);
        return;
    }
    else if (strcmp(message->content, "chat_request") == 0)
    {
        std::cout << "Received chat request from " << message->sender << std::endl;
        if (_chats.find(message->sender) != _chats.end())
        {
            std::cout << "Chat with " << message->sender << " already exists. Ignoring request." << std::endl;
            // Send control message to accept the chat
            std::string topic = "dev/marconautas/user/" + std::string(message->sender) + "/control";
            Message controlMsg;
            controlMsg.type = MessageType::PRIVATE_REQUEST;
            strcpy(controlMsg.sender, _client->getClientId().c_str());
            strcpy(controlMsg.content, "chat_accepted");
            controlMsg.timestamp = std::chrono::system_clock::now();
            _client->publish(topic.c_str(), controlMsg);
            return;
        }
        _pendingRequestsFrom[message->sender] = true;
    }
}

void App::onChatCreationMessage(const char *topic, Message *message)
{

    std::string groupName = std::string(topic).substr(std::string("dev/marconautas/group/").length());
    if (_groupChats.find(groupName) == _groupChats.end())
    {
        _allGroups[groupName] = GroupChat();
        _allGroups[groupName].groupName = groupName;
        _allGroups[groupName].owner = std::string(message->sender);
        _allGroups[groupName].isOpen = false;
        std::cout << "New group chat created: " << groupName << " by " << message->sender << std::endl;
    }
}
