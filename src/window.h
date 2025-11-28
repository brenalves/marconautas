#pragma once

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <queue>
#include <stb_image.h>

#include "client.h"

struct PrivateChat
{
    std::queue<Message> messages;
    char draft[256]; // Draft message being composed
    bool isOpen = true; // Is the chat tab open
};

struct GroupChat
{
    std::queue<Message> messages;
    char draft[256]; // Draft message being composed
    bool isOpen = true; // Is the chat tab open
    std::string groupName; // Name of the group
    std::string owner; // Owner of the group
};

class Window
{
public:
    Window(int width, int height, const char* title);
    ~Window();

    inline void setOnCloseButtonClick(const std::function<void()>& callback) { _onCloseButtonClick = callback; }
    inline void setOnLoginButtonSubmit(const std::function<void(const char*)>& callback) { _onLoginButtonSubmit = callback; }
    inline void setOnChatRequestClick(const std::function<void(const char*)>& callback) { _onChatRequestClick = callback; }
    inline void setOnChatRequestAccept(const std::function<void(const char*)>& callback) { _onChatRequestAccept = callback; }
    inline void setOnChatRequestDecline(const std::function<void(const char*)>& callback) { _onChatRequestDecline = callback; }
    inline void setOnSendMessage(const std::function<void(const char*, const char*, int)>& callback) { _onSendMessage = callback; }
    inline void setOnCreateGroupClick(const std::function<void(const char*)>& callback) { _onCreateGroupClick = callback; }

    inline void setActiveUsersVector(std::vector<std::string>* users) { _activeUsers = users; }
    inline void setChatsMap(std::unordered_map<std::string, PrivateChat>* chats) { _chats = chats; }
    inline void setGroupChatsMap(std::unordered_map<std::string, GroupChat>* groupChats) { _groupChats = groupChats; }
    inline void setPendingRequestsFromMap(std::unordered_map<std::string, bool>* requests) { _pendingRequestsFrom = requests; }
    inline void setPendingRequestsToMap(std::unordered_map<std::string, bool>* requests) { _pendingRequestsTo = requests; }
    inline void setGroupRequestsFromMap(std::unordered_map<std::string, bool>* requests) { _groupRequestFrom = requests; }
    inline void setGroupRequestsToMap(std::unordered_map<std::string, bool>* requests) { _groupRequestTo = requests; }
    inline void setAllGroupsMap(std::unordered_map<std::string, GroupChat>* allGroups) { _allGroups = allGroups; }

    void beginFrame();
    void showLogin();
    void showMain();
    void endFrame();

    void setTitle(const char* title);
    void setUsername(const char* username);

private:
    GLFWwindow* _window;
    int _width;
    int _height;
    const char* _title;

    char _username[64];
    char _groupName[64];

    std::function<void()> _onCloseButtonClick;
    std::function<void(const char*)> _onLoginButtonSubmit;
    std::function<void(const char*)> _onChatRequestClick;
    std::function<void(const char*)> _onChatRequestAccept;
    std::function<void(const char*)> _onChatRequestDecline;
    std::function<void(const char*, const char*, int)> _onSendMessage;
    std::function<void(const char*)> _onCreateGroupClick;

    std::vector<std::string>* _activeUsers;
    std::unordered_map<std::string, PrivateChat>* _chats;
    std::unordered_map<std::string, GroupChat>* _groupChats;
    std::unordered_map<std::string, GroupChat>* _allGroups;
    std::unordered_map<std::string, bool>* _pendingRequestsFrom;
    std::unordered_map<std::string, bool>* _pendingRequestsTo;
    std::unordered_map<std::string, bool>* _groupRequestFrom;
    std::unordered_map<std::string, bool>* _groupRequestTo;
};