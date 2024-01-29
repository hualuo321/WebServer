#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

// 线程池类
class ThreadPool {
public:
    // 构造函数
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0);
            // 创建指定数量的线程
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {            // lambda 匿名函数, 闭包
                    // 获取线程池中的锁
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while(true) {
                        if(!pool->tasks.empty()) {
                            // 如果任务队列不为空，则执行任务
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();            // 加锁
                            task();                     // 处理任务
                            locker.lock();              // 解锁
                        } 
                        else if(pool->isClosed) break;  // 如果线程池关闭，则退出
                        else pool->cond.wait(locker);   // 等待条件变量
                    }
                }).detach();                            // 线程分离
            }
    }
    // 默认构造函数
    ThreadPool() = default;
    // 移动构造函数
    ThreadPool(ThreadPool&&) = default;
    // 析构函数
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;                 // 标记线程池关闭
            }
            pool_->cond.notify_all();                   // 通知所有等待的线程
        }
    }

    // 添加任务, 将阻塞的条件变量唤醒
    template<class F>
    void AddTask(F&& task) {
        {
            // 将任务添加到队列
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();                       // 通知一个等待的线程
    }

private:
    // 线程池
    struct Pool {
        std::mutex mtx;                                 // 互斥锁
        std::condition_variable cond;                   // 条件变量
        bool isClosed;                                  // 线程池是否关闭标志
        std::queue<std::function<void()>> tasks;        // 任务队列
    };
    std::shared_ptr<Pool> pool_;                        // 指向线程池的共享指针
};

#endif //THREADPOOL_H