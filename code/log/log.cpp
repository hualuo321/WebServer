#include "log.h"

using namespace std;

// 构造函数
Log::Log() {
    lineCount_ = 0;                         // 初始化行数计数器为 0
    isAsync_ = false;                       // 初始化异步标志为 false
    writeThread_ = nullptr;                 // 初始化写入线程 unique_ptr 为空
    deque_ = nullptr;                       // 初始化日志队列 unique_ptr 为空
    toDay_ = 0;                             // 初始化日期为 0
    fp_ = nullptr;                          // 初始化 FILE 指针为空
}

// 析构函数
Log::~Log() {
    // 如果写入线程存在且可以被 join
    if(writeThread_ && writeThread_->joinable()) {  
        while(!deque_->empty()) {           // 当日志队列不为空时
            deque_->flush();                // 刷新队列, 等待队列为空
        };
        deque_->Close();                    // 关闭队列
        writeThread_->join();               // 等待与 writeThread_ 关联的线程完成其执行
    }
    if(fp_) {                               // 如果文件指针不为空
        lock_guard<mutex> locker(mtx_);     // 加锁
        flush();                            // 刷新输出缓冲区
        fclose(fp_);                        // 关闭文件
    }
}

// 获取日志等级的方法
int Log::GetLevel() {
    lock_guard<mutex> locker(mtx_);         // 加锁
    return level_;                          // 返回当前日志等级
}

// 设置日志等级的方法
void Log::SetLevel(int level) {
    lock_guard<mutex> locker(mtx_);         // 加锁
    level_ = level;                         // 设置当前日志等级
}

// 初始化日志系统
void Log::init(int level = 1, const char* path, const char* suffix, int maxQueueSize) {
    isOpen_ = true;                         // 设置日志系统为开启状态
    level_ = level;                         // 设置日志等级

    if(maxQueueSize > 0) {
        // 如果日志队列 > 0，说明采用了异步模式, 同步模式的话不需要日志队列
        isAsync_ = true;
        if(!deque_) {                       // 如果队列不存在，则创建
            unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            deque_ = move(newDeque);
            // 创建一个写入线程
            std::unique_ptr<std::thread> NewThread(new thread(FlushLogThread));
            writeThread_ = move(NewThread);
        }
    } else {
        // 日志队列大小为 0，表示设置为同步模式
        isAsync_ = false;
    }

    lineCount_ = 0;                         // 行计数器重置为 0
    time_t timer = time(nullptr);           // 获取当前时间，并格式化文件名
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path_ = path;                           // 设置文件路径
    suffix_ = suffix;                       // 设置文件后缀
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;
    {
        lock_guard<mutex> locker(mtx_);     // 加锁, 出大括号作用域会调用析构函数自动解锁
        buff_.RetrieveAll();                // 清空缓冲区
        if(fp_) {
            flush();                        // 刷新日志缓冲区, 处理异步模式下的日志队列
            fclose(fp_);                    // 关闭文件
        }

        // 打开或创建文件
        fp_ = fopen(fileName, "a");
        if(fp_ == nullptr) {
            // 如果文件夹不存在，则创建
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        } 
        assert(fp_ != nullptr);
    }
}

// 写入日志的函数
void Log::write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);            // 获取当前时间
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);  // 转换为本地时间
    struct tm t = *sysTime;
    va_list vaList;

    // 日志日期, 日志行数是否满足要求
    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_  %  MAX_LINES == 0))) {
        // 检查是否需要创建新的日志文件（日期变化或行数超过最大值）
        unique_lock<mutex> locker(mtx_);    // 加锁
        
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        // 根据日期变化或行数超限制来更新文件名
        if (toDay_ != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_  / MAX_LINES), suffix_);
        }
        
        flush();                            // 刷新缓冲区
        fclose(fp_);                        // 关闭文件
        fp_ = fopen(newFile, "a");          // 确保文件不会为空, 追加打开文件
        assert(fp_ != nullptr);
    }

    {
        unique_lock<mutex> locker(mtx_);    // 加锁
        lineCount_++;                       // 增加行数计数
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
                    
        buff_.HasWritten(n);                // 更新缓冲区的写入位置
        AppendLogLevelTitle_(level);        // 添加日志级别标题

        va_start(vaList, format);           // 开始处理可变参数
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);                     // 结束处理可变参数

        buff_.HasWritten(m);                // 更新缓冲区写入位置
        buff_.Append("\n\0", 2);            // 添加换行和字符串结束符

        if(isAsync_ && deque_ && !deque_->full()) {
            // 如果是异步模式且队列未满，则将日志信息添加到队列中
            deque_->push_back(buff_.RetrieveAllToStr());
        } else {
            // 否则直接写入文件
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll();                        // 清空缓冲区
    }
}

// 根据日志级别添加日志标题
void Log::AppendLogLevelTitle_(int level) {
    switch(level) {
    case 0:
        buff_.Append("[debug]: ", 9);       // 如果级别为0，添加"[debug]: "到缓冲区
        break;
    case 1:
        buff_.Append("[info] : ", 9);       // 如果级别为1，添加"[info] : "到缓冲区
        break;
    case 2:
        buff_.Append("[warn] : ", 9);       // 如果级别为2，添加"[warn] : "到缓冲区
        break;
    case 3:
        buff_.Append("[error]: ", 9);       // 如果级别为3，添加"[error]: "到缓冲区
        break;
    default:
        buff_.Append("[info] : ", 9);       // 默认情况下，添加"[info] : "到缓冲区
        break;
    }
}

// 刷新日志缓冲区, 处理异步模式下的日志队列
void Log::flush() {
    if(isAsync_) {
        // 如果是异步模式，通知消费者有数据可读
        deque_->flush(); 
    }
    // 清空文件流的输出缓冲区，确保所有缓存的日志数据都被写入到与 fp_ 相关联的文件中
    fflush(fp_);
}

// 异步写入日志
void Log::AsyncWrite_() {
    string str = "";
    while(deque_->pop(str)) {               // 从日志队列中取出字符串
        lock_guard<mutex> locker(mtx_);     // 
        fputs(str.c_str(), fp_);            // 将日志数据写入文件中 (通过 FILE* 文件句柄写入)
    }
}

// 获取一个日志单例, 被所有类共享
Log* Log::Instance() {
    static Log inst;
    return &inst;
}

// 刷新写入日志的线程, 异步写入
void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite_();
}