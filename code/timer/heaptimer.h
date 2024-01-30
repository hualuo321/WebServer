#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;      // 定义回调函数类型
typedef std::chrono::high_resolution_clock Clock;   // 使用高精度时钟
typedef std::chrono::milliseconds MS;               // 毫秒单位
typedef Clock::time_point TimeStamp;                // 时间点类型

// 定时器节点结构
struct TimerNode {
    int id;                                         // 定时器标识
    TimeStamp expires;                              // 到期时间
    TimeoutCallBack cb;                             // 到期回调
    bool operator<(const TimerNode& t) {            // 比较运算符，用于确定优先级
        return expires < t.expires;
    }
};

// 堆定时器类
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }              // 构造函数
    ~HeapTimer() { clear(); }                       // 析构函数
    
    void adjust(int id, int newExpires);            // 调整定时器
    // 添加定时器
    void add(int id, int timeOut, const TimeoutCallBack& cb);
    void doWork(int id);
    void clear();
    void tick();
    void pop();
    int GetNextTick();
private:
    void del_(size_t i);
    void siftup_(size_t i);                         // 向上调整堆
    bool siftdown_(size_t index, size_t n);         // 向下调整堆
    void SwapNode_(size_t i, size_t j);             // 交换节点
    
    std::vector<TimerNode> heap_;                   // 定时器堆
    std::unordered_map<int, size_t> ref_;           // 定时器引用，用于快速定位
};

#endif //HEAP_TIMER_H