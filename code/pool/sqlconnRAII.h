#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H
#include "sqlconnpool.h"

// SqlConnRAII 类用于自动管理 SQL 连接的获取与释放
class SqlConnRAII {
public:
    // 构造函数
    SqlConnRAII(MYSQL** sql, SqlConnPool *connpool) {
        assert(connpool);                       // 断言连接池不为空
        *sql = connpool->GetConn();             // 从连接池获取一个数据库连接
        sql_ = *sql;                            // 保存获取的连接
        connpool_ = connpool;                   // 保存连接池的引用
    }
    // 析构函数
    ~SqlConnRAII() {
        if(sql_) { connpool_->FreeConn(sql_); } // 释放连接
    }
    
private:
    MYSQL *sql_;                                // SQL连接指针
    SqlConnPool* connpool_;                     // 连接池对象的指针
};

#endif //SQLCONNRAII_H