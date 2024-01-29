#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

// SQL连接池类
class SqlConnPool {
public:
    static SqlConnPool *Instance();                 // 获取单例对象
    MYSQL *GetConn();                               // 获取一个数据库连接
    void FreeConn(MYSQL * conn);                    // 释放一个数据库连接
    int GetFreeConnCount();                         // 获取空闲连接数
    // 初始化连接池
    void Init(const char* host, int port, const char* user,const char* pwd, const char* dbName, int connSize);
    // 关闭连接池
    void ClosePool();

private:
    // 构造函数
    SqlConnPool();
    // 析构函数
    ~SqlConnPool();
    // 最大连接数
    int MAX_CONN_;
    // 当前使用的连接数
    int useCount_;
    // 空闲的连接数
    int freeCount_;

    std::queue<MYSQL *> connQue_;                   // 连接队列
    std::mutex mtx_;                                // 互斥锁
    sem_t semId_;                                   // 信号量
};

#endif // SQLCONNPOOL_H