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

struct Chat
{
    std::queue<Message> messages;
    char draft[256]; // Draft message being composed
    bool isOpen = true; // Is the chat tab open
    bool isGroup = false; // Is this chat a group chat
};

class Window
{
public:
    Window(int width, int height, const char* title);
    ~Window();

    inline void setOnCloseButtonClick(const std::function<void()>& callback) { _onCloseButtonClick = callback; }
    inline void setOnLoginButtonSubmit(const std::function<void(const char*)>& callback) { _onLoginButtonSubmit = callback; }
    inline void setOnChatRequestClick(const std::function<void(const char*)>& callback) { _onChatRequestClick = callback; }
    inline void setOnSendMessage(const std::function<void(const char*, const char*)>& callback) { _onSendMessage = callback; }
    
    inline void setActiveUsersVector(std::vector<std::string>* users) { _activeUsers = users; }
    inline void setChatsMap(std::unordered_map<std::string, Chat>* chats) { _chats = chats; }

    void beginFrame();
    void showLogin();
    void showMain();
    void endFrame();

    void setTitle(const char* title);

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
    std::function<void(const char*, const char*)> _onSendMessage;

    std::vector<std::string>* _activeUsers;
    std::unordered_map<std::string, Chat>* _chats;
};