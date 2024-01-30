#include "sqlconnpool.h"
using namespace std;

// 构造函数
SqlConnPool::SqlConnPool() {
    useCount_ = 0;                              // 已使用的连接数
    freeCount_ = 0;                             // 空闲的连接数
}

// 获取单例实例
SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool connPool;                // 静态局部变量，确保只创建一次
    return &connPool;
}

// 初始化连接池
void SqlConnPool::Init(const char* host, int port, const char* user,const char* pwd, const char* dbName, int connSize = 10) {
    assert(connSize > 0);
    // 初始化指定数量的 MySQL 连接
    for (int i = 0; i < connSize; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);                  // 初始化 MySQL 对象
        if (!sql) {
            LOG_ERROR("MySql init error!");     
            assert(sql);                        
        }
        // 连接MySQL数据库
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySql Connect error!");
        }
        connQue_.push(sql);                     // 将连接的 MySQL 放入连接池
    }
    MAX_CONN_ = connSize;                       // 设置最大连接数
    sem_init(&semId_, 0, MAX_CONN_);            // 初始化信号量 (初始为 MAX)
}

// 获取一个连接
MYSQL* SqlConnPool::GetConn() {
    MYSQL *sql = nullptr;
    if(connQue_.empty()){
        LOG_WARN("SqlConnPool busy!");          // 日志记录连接池忙碌
        return nullptr;
    }
    sem_wait(&semId_);                          // 等待信号量
    {
        lock_guard<mutex> locker(mtx_);         // 加锁
        sql = connQue_.front();                 // 获取队列头部的连接
        connQue_.pop();                         // 弹出队列头部的连接
    }
    return sql;                                 // 返回连接
}

// 释放一个连接
void SqlConnPool::FreeConn(MYSQL* sql) {
    assert(sql);                                // 断言连接非空
    lock_guard<mutex> locker(mtx_);             // 加锁
    connQue_.push(sql);                         // 将连接放回队列
    sem_post(&semId_);                          // 释放信号量
}

// 关闭连接池
void SqlConnPool::ClosePool() {
    lock_guard<mutex> locker(mtx_);             // 加锁
    while(!connQue_.empty()) {                  
        auto item = connQue_.front();           // 获取队列头部的连接
        connQue_.pop();                         // 弹出队列头部的连接
        mysql_close(item);                      // 关闭MySQL连接
    }
    mysql_library_end();        
}

// 获取空闲连接数
int SqlConnPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mtx_);             // 加锁
    return connQue_.size();                     // 返回队列中的连接数
}

// 析构函数
SqlConnPool::~SqlConnPool() {
    ClosePool();                                // 关闭连接池
}
