#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include "blockqueue.h"
#include "../buffer/buffer.h"

// Log 类
class Log {
public:
    // 初始化日志系统，设置日志等级、路径、文件后缀和队列最大容量
    void init(int level, const char* path = "./log", const char* suffix =".log", int maxQueueCapacity = 1024);
    // 获取日志系统的单例
    static Log* Instance();
    // 启动日志刷新线程
    static void FlushLogThread();
    // 写日志，支持格式化输出
    void write(int level, const char *format,...);
    // 刷新日志，将日志内容输出到文件
    void flush();
    // 获取日志等级
    int GetLevel();
    // 设置日志等级
    void SetLevel(int level);
    // 判断日志系统是否已开启
    bool IsOpen() { return isOpen_; }
    
private:
    Log();
    virtual ~Log();
    // 内部函数，添加日志等级标题
    void AppendLogLevelTitle_(int level);
    // 异步写日志
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;    // 路径长度
    static const int LOG_NAME_LEN = 256;    // 文件名长度
    static const int MAX_LINES = 50000;     // 单文件记录最大行数

    const char* path_;                      // 日志路径
    const char* suffix_;                    // 日期后缀
    int MAX_LINES_;                         // 最大行数
    int lineCount_;                         // 当前行数和日期
    int toDay_;
    bool isOpen_;                           // 是否开启日志系统
    Buffer buff_;                           // 缓冲区
    int level_;                             // 日志等级
    bool isAsync_;                          // 是否异步

    // FILE 文件指针结构体 (其中包含 fp, 文件位置指针, 缓冲区等信息)
    FILE* fp_;
    // 日志队列，用于存储日志消息
    std::unique_ptr<BlockDeque<std::string>> deque_;
    // 写日志的线程
    std::unique_ptr<std::thread> writeThread_;
    // 互斥锁，用于线程同步
    std::mutex mtx_;
};

// 宏定义，用于不同等级的日志输出
#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H