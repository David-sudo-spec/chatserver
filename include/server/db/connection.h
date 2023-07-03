#ifndef CPP_ADVANCE_CHAT_SERVER_CONNECTION_H
#define CPP_ADVANCE_CHAT_SERVER_CONNECTION_H

#include <mysql/mysql.h>
#include <string>
#include <ctime>

using std::string;

// 数据库操作类
class Connection
{
public:
    // 初始化数据库连接
    Connection();

    // 释放数据库连接资源
    ~Connection();

    // 连接数据库
    bool connect(const string &ip, unsigned short port, const string &username, const string &password, const string &dbname);

    // 更新操作
    bool update(const string &sql);

    // 查询操作
    MYSQL_RES *query(const string &sql);

    // 刷新一下连接的起始的空闲时间点
    void refreshAliveTime() { _aliveTime = clock(); }

    // 返回存活的时间
    clock_t getAliveTime() const { return clock() - _aliveTime; }

    // 返回Mysql的连接
    MYSQL *getConn() { return _conn; }
private:
    MYSQL *_conn;           // 表示和MYSQL的一条连接
    clock_t _aliveTime;     // 记录进入空闲状态后的起始时间
};

#endif //CPP_ADVANCE_CHAT_SERVER_CONNECTION_H
