#ifndef CPP_ADVANCE_CHAT_SERVER_GROUPUSER_H
#define CPP_ADVANCE_CHAT_SERVER_GROUPUSER_H

#include "user.h"

// 群组用户，多了一个role角色信息，从User类直接继承，复用User的其他信息
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return this->role; }
private:
    string role;
};

#endif //CPP_ADVANCE_CHAT_SERVER_GROUPUSER_H
