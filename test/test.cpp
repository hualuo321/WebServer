#include "../code/log/log.h"
#include "../code/pool/threadpool.h"
#include <features.h>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

// 定义一个测试日志记录的函数
void TestLog() {
    int cnt = 0, level = 0;
    // 初始化日志实例，设置日志等级、文件名前缀、文件名后缀和文件大小限制
    Log::Instance()->init(level, "./testlog1", ".log", 0);
    // 循环设置不同的日志等级，并生成大量日志消息
    for(level = 3; level >= 0; level--) {
        Log::Instance()->SetLevel(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                // 生成日志消息，日志等级为i，内容包括文本和计数器
                LOG_BASE(i,"%s 111111111 %d ============= ", "Test", cnt++);
            }
        }
    }
    cnt = 0;
    // 重新初始化日志实例，设置新的文件名和文件大小限制
    Log::Instance()->init(level, "./testlog2", ".log", 5000);
    // 与上面类似，但是日志文件和内容不同
    for(level = 0; level < 4; level++) {
        Log::Instance()->SetLevel(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"%s 222222222 %d ============= ", "Test", cnt++);
            }
        }
    }
}

// 定义一个线程任务，用于生成日志记录
void ThreadLogTask(int i, int cnt) {
    for(int j = 0; j < 10000; j++ ){
        // 生成日志消息，包括线程ID和计数器
        LOG_BASE(i,"PID:[%04d]======= %05d ========= ", gettid(), cnt++);
    }
}

// 定义一个测试线程池的函数
void TestThreadPool() {
    // 初始化日志实例
    Log::Instance()->init(0, "./testThreadpool", ".log", 5000);
    // 创建一个包含6个线程的线程池
    ThreadPool threadpool(6);
    // 向线程池添加任务
    for(int i = 0; i < 18; i++) {
        threadpool.AddTask(std::bind(ThreadLogTask, i % 4, i * 10000));
    }
    // 等待用户输入以继续执行
    getchar();
}

// 主函数，执行日志记录和线程池测试
int main() {
    TestLog();
    TestThreadPool();
}