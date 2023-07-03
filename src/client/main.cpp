#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "group.h"
#include "user.h"
#include "public.h"

using namespace std;
using json = nlohmann::json;

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;
// 控制主菜单页面程序运行
bool isMainMenuRunning = false;

// 显示当前登录成功用户的基本信息
void showCurrentUserData();
// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
// 主聊天界面
void mainMenu(int);

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接收线程
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 8888" << endl;
        exit(1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建client端的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket create error" << endl;
        exit(1);
    }

    // 填写client需要连接的server信息ip+port
    sockaddr_in server;
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    // client和server进行连接
    if (connect(clientfd, (struct sockaddr *) &server, sizeof(sockaddr_in)) == -1)
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(1);
    }

    // main线程用于接收用户输入，负责发送数据
    for (;;)
    {
        // 显示首页面菜单  登录、注册、退出
        cout << "=====================================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "=====================================" << endl;
        cout << "choice: ";
        int choice = 0;
        cin >> choice;
        cin.get();      // 读出缓冲区残留的回车

        switch (choice)
        {
            case 1: // login 业务
            {
                int id = 0;
                char pwd[50] = {0};
                cout << "userId: ";
                cin >> id;
                cin.get();
                cout << "userPassword: ";
                cin.getline(pwd, 50);

                json js;
                js["msgId"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1)
                {
                    cerr << "send login msg error:" << request << endl;
                }
                else
                {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if (len == -1)
                    {
                        cerr << "recv login response error:" << request << endl;
                    }
                    else
                    {
                        json responseJson = json::parse(buffer);
                        if (responseJson["errno"].get<int>() != 0)  // 登录失败
                        {
                            cerr << responseJson["errMsg"] <<  endl;
                        }
                        else    // 登录失败
                        {
                            // 记录当前用户的id和name
                            g_currentUser.setId(responseJson["id"].get<int>());
                            g_currentUser.setName(responseJson["name"]);

                            // 记录当前好友的好友列表消息
                            if (responseJson.contains("friends"))
                            {
                                // 初始化
                                g_currentUserFriendList.clear();

                                vector<string> vec = responseJson["friends"];
                                for (string &str : vec)
                                {
                                    json userJson = json::parse(str);
                                    User user;
                                    user.setId(userJson["id"].get<int>());
                                    user.setName(userJson["name"]);
                                    user.setState(userJson["state"]);
                                    g_currentUserFriendList.push_back(user);
                                }
                            }

                            // 记录当前用户的群组列表信息
                            if (responseJson.contains("groups"))
                            {
                                // 初始化
                                g_currentUserGroupList.clear();

                                vector<string> vec1 = responseJson["groups"];
                                for (string &groupStr : vec1)
                                {
                                    json grpJson = json::parse(groupStr);
                                    Group group;
                                    group.setId(grpJson["id"].get<int>());
                                    group.setName(grpJson["groupName"]);
                                    group.setDesc(grpJson["groupDesc"]);

                                    vector<string> vec2 = grpJson["users"];
                                    for (string &userStr : vec2)
                                    {
                                        GroupUser groupUser;
                                        json grpUsrJson = json::parse(userStr);
                                        groupUser.setId(grpUsrJson["id"].get<int>());
                                        groupUser.setName(grpUsrJson["name"]);
                                        groupUser.setState(grpUsrJson["state"]);
                                        groupUser.setRole(grpUsrJson["role"]);
                                        group.getUsers().push_back(groupUser);
                                    }
                                    g_currentUserGroupList.push_back(group);
                                }
                            }

                            // 显示登录用户的基本信息
                            showCurrentUserData();

                            // 显示当前用户的离线消息  个人聊天信息或者群组消息
                            if (responseJson.contains("offlineMsg"))
                            {
                                vector<string> vec = responseJson["offlineMsg"];
                                for (string &str : vec)
                                {
                                    json msgJson = json::parse(str);
                                    // time + [id] + name + " said:" + xxx
                                    if (ONE_CHAT_MSG == msgJson["msgId"].get<int>())
                                    {
                                        cout << msgJson["time"].get<string>() << " [" << msgJson["id"] << "]"
                                             << msgJson["name"].get<string>()
                                             << " said: " << msgJson["msg"].get<string>() << endl;
                                    }
                                    else
                                    {
                                        cout << "群消息[" << msgJson["groupId"] << "]:" << msgJson["time"].get<string>()
                                             << " [" << msgJson["id"] << "]" << msgJson["name"].get<string>()
                                             << " said: " << msgJson["msg"].get<string>() << endl;
                                    }
                                }
                            }

                            // 登录成功，启动接受线程负责接收数据,该线程只启动一次
                            static int readThreadNumber = 0;
                            if (readThreadNumber == 0)
                            {
                                std::thread readTask(readTaskHandler, clientfd);
                                readTask.detach();
                                readThreadNumber++;
                            }

                            // 进入聊天主菜单页面
                            isMainMenuRunning = true;
                            mainMenu(clientfd);
                        }
                    }
                }
                break;
            }
            case 2:     // 注册业务
            {
                char name[50] = {0};
                char pwd[50] = {0};
                cout << "username: ";
                cin.getline(name, 50);
                cout << "userPassword: ";
                cin.getline(pwd, 50);

                json js;
                js["msgId"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1)
                {
                    cerr << "send reg msg error:" << request << endl;
                }
                else
                {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if (len == -1)
                    {
                        cerr << "recv reg response error:" << request << endl;
                    }
                    else
                    {
                        json responseJson = json::parse(buffer);
                        if (responseJson["errno"].get<int>() != 0)  // 注册失败
                        {
                            cerr << name << " is already exist, register error!" << endl;
                        }
                        else    // 注册成功
                        {
                            cout << name << " register success, userId is " << responseJson["id"]
                                 << ", do not forget it!" << endl;
                        }
                    }
                }
                break;
            }
            case 3:     // quit业务
                close(clientfd);
                exit(1);
            default:
                cerr << "invalid input!" << endl;
                break;
        }
    }
    return 0;
}

// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

// 接收线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (len == -1 || len == 0)
        {
            close(clientfd);
            exit(1);
        }

        json js = json::parse(buffer);
        int msgType = js["msgId"].get<int>();
        if (msgType == ONE_CHAT_MSG)
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (msgType == GROUP_CHAT_MSG)
        {
            cout << "群消息[" << js["groupId"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
    }
}

// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}

// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addFriend" command handler
void addFriend(int, string);
// "createGroup" command handler
void createGroup(int, string);
// "addGroup" command handler
void addGroup(int, string);
// "groupChat" command handler
void groupChat(int, string);
// "logout" command handler
void logout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
        {"help", "显示所有支持的命令，格式help"},
        {"chat", "一对一聊天，格式chat:friendId:message"},
        {"addFriend", "添加好友，格式addFriend:friendId"},
        {"createGroup", "创建群组，格式createGroup:groupName:groupDesc"},
        {"addGroup", "加入群组，格式addGroup:groupId"},
        {"groupChat", "群聊，格式groupChat:groupId:message"},
        {"logout", "注销，格式logout"}};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
        {"help", help},
        {"chat", chat},
        {"addFriend", addFriend},
        {"createGroup", createGroup},
        {"addGroup", addGroup},
        {"groupChat", groupChat},
        {"logout", logout}};

// 主聊天界面
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandBuf(buffer);
        string command;        // 存储命令
        int idx = commandBuf.find(":");
        if (idx == -1)
            command = commandBuf;
        else
            command = commandBuf.substr(0, idx);
        auto it = commandHandlerMap.find(command);
        if (commandHandlerMap.end() == it)
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd, commandBuf.substr(idx + 1, commandBuf.size() - idx));
    }
}

// "help" command handler
void help(int, string)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

// "chat" command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendId = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgId"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toId"] = friendId;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

// "addFriend" command handler
void addFriend(int clientfd, string str)
{
    int friendId = atoi(str.c_str());
    json js;
    js["msgId"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendId"] = friendId;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addFriend msg error -> " << buffer << endl;
    }
}

// "createGroup" command handler
void createGroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "createGroup command invalid!" << endl;
        return;
    }

    string groupName = str.substr(0, idx);
    string groupDesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgId"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupName"] = groupName;
    js["groupDesc"] = groupDesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send createGroup msg error -> " << buffer << endl;
    }
}

// "addGroup" command handler
void addGroup(int clientfd, string str)
{
    int groupId = atoi(str.c_str());
    json js;
    js["msgId"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupId"] = groupId;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addGroup msg error -> " << buffer << endl;
    }
}

// "groupChat" command handler
void groupChat(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "groupChat command invalid!" << endl;
        return;
    }

    int groupId = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgId"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupId"] = groupId;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send groupChat msg error -> " << buffer << endl;
    }
}

// "logout" command handler
void logout(int clientfd, string str)
{
    json js;
    js["msgId"] = LOGOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send logout msg error -> " << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}