#include "connection.h"
#include <muduo/base/Logging.h>

// 初始化数据库连接
Connection::Connection()
{
    _conn = mysql_init(nullptr);
}

// 释放数据库连接资源
Connection::~Connection()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}

// 连接数据库
bool Connection::connect(const string &ip, unsigned short port, const string &username, const string &password, const string &dbname)
{
    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
                                  password.c_str(), dbname.c_str(), port, nullptr, 0);
    if (p != nullptr)
    {
        // C和C++代码默认的编码字符是ASCII，如果不设置，从MYSQL上拉下来的中文显示？？
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
    }
    return p;
}

// 更新操作
bool Connection::update(const string &sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
        return false;
    }
    return true;
}

// 查询操作
MYSQL_RES *Connection::query(const string &sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}