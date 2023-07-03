#ifndef CPP_ADVANCE_CHAT_SERVER_GROUPMODEL_H
#define CPP_ADVANCE_CHAT_SERVER_GROUPMODEL_H

#include "group.h"
#include <string>
#include <vector>

using namespace std;

// 维护群组信息的操作接口方法
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);

    // 加入群组
    void addGroup(int userId, int groupId, string role);

    // 查询用户所在群组信息
    vector<Group> queryGroups(int userId);

    // 根据指定的groupId查询群组用户id列表，除userId自己，主要用户群聊业务给群组其它成员群发消息
    vector<int> queryGroupUsers(int userId, int groupId);
};

#endif //CPP_ADVANCE_CHAT_SERVER_GROUPMODEL_H
