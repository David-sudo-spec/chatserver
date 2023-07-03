#include <offlinemessagemodel.h>
#include "connectionpool.h"

// 存储用户的离线消息
void OfflineMsgModel::insert(int userId, string msg)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d,'%s')", userId, msg.c_str());
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    auto sp = cp->getConnection();
    sp->update(sql);
}

// 删除用户的离线消息
void OfflineMsgModel::remove(int userId)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userId);
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    auto sp = cp->getConnection();
    sp->update(sql);
}

// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userId)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userId);
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    auto sp = cp->getConnection();

    vector<string> vec;
    MYSQL_RES *res = sp->query(sql);
    if (res != nullptr)
    {
        // 把userId用户的所有离线消息放入vec中返回
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
            vec.emplace_back(row[0]);
        mysql_free_result(res);
        return vec;
    }
    return vec;
}
