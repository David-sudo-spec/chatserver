#ifndef CPP_ADVANCE_CHAT_SERVER_CONNECTIONPOOL_H
#define CPP_ADVANCE_CHAT_SERVER_CONNECTIONPOOL_H

#include "connection.h"
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>

using std::string;

/**
 * 实现连接池功能模块
 */
class ConnectionPool
{
public:
    // 获取链接池对象实例
    static ConnectionPool *getConnectionPool();

    // 给外部提供接口，从连接池中获取一个可用的空闲连接
    std::shared_ptr<Connection> getConnection();
private:
    // 构造函数私有化
    ConnectionPool();

    // 从配置文件中加载配置项
    bool loadConfigFile();

    // 运行在独立的线程中，专门负责生产新连接
    void produceConnectionTask();

    // 扫描超过maxIdleTime时间的空闲连接，进行队列的连接回收
    void scannerConnectionTask();

    string _ip;                 // mysql的ip地址
    unsigned short _port;       // mysql的端口号3306
    string _username;           // mysql的登陆用户名
    string _password;           // mysql登录密码
    string _dbname;             // 连接的数据库名称
    int _initSize;              // 连接池的初始连接量
    int _maxSize;               // 连接池的最大连接量
    int _maxIdleTime;           // 连接池最大空闲时间
    int _connectionTimeout;     // 连接池获取链接的超时时间

    std::queue<Connection *> _connectionQue;   // 存储mysql连接的队列
    std::mutex _queueMutex;     // 维护连接队列的线程安全互斥锁
    std::atomic_int _connectionCnt;            // 记录连接所创建的connection连接的总数量
    std::condition_variable cv; // 设置条件变量，用于连接生产线程和消费线程的通信
};

#endif //CPP_ADVANCE_CHAT_SERVER_CONNECTIONPOOL_H
