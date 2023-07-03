#ifndef CPP_ADVANCE_CHAT_SERVER_FRIENDMODEL_H
#define CPP_ADVANCE_CHAT_SERVER_FRIENDMODEL_H

#include "user.h"
#include <vector>

using namespace std;

// 维护好友信息的接口类
class FriendModel
{
public:
    // 添加好友关系
    void insert(int userId, int friendId);

    // 返回用户好友列表
    vector<User> query(int userId);
};

#endif //CPP_ADVANCE_CHAT_SERVER_FRIENDMODEL_H
