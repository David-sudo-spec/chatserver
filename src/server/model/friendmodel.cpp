#include "friendmodel.h"
#include "connectionpool.h"

// 添加好友关系
void FriendModel::insert(int userId, int friendId)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d)", userId, friendId);
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    auto sp = cp->getConnection();
    sp->update(sql);
}

// 返回用户好友列表
vector<User> FriendModel::query(int userId)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from user a inner join friend b on b.friendid = a.id where b.userid = %d", userId);
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    auto sp = cp->getConnection();

    vector<User> vec;
    MYSQL_RES *res = sp->query(sql);
    if (res != nullptr)
    {
        // 把userId用户的所有离线消息放入vec中返回
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setState(row[2]);
            vec.push_back(user);
        }
        mysql_free_result(res);
        return vec;
    }
    return vec;
}