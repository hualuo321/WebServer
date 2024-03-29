#include "heaptimer.h"

// 向上调整堆
void HeapTimer::siftup_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;                             // 父节点索引
    // 当不是根节点且当前节点小于父节点时，向上调整
    while(j >= 0) {
        if(heap_[j] < heap_[i]) { break; }
        SwapNode_(i, j);                                // 交换节点
        i = j;
        j = (i - 1) / 2;
    }
}

// 交换堆中两个节点
void HeapTimer::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);                      // 交换元素
    ref_[heap_[i].id] = i;                              // 更新哈希表
    ref_[heap_[j].id] = j;
} 

// 向下调整堆
bool HeapTimer::siftdown_(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;                               // 子节点索引
    // 调整堆直到节点成为叶子节点
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

// 添加定时器
void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;
    if(ref_.count(id) == 0) {
        // 如果是新节点，则添加到堆的末尾，并调整堆
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        siftup_(i);
    } 
    else {
        // 如果已存在，则调整堆
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if(!siftdown_(i, heap_.size())) {
            siftup_(i);
        }
    }
}

// 执行指定id的定时器任务
void HeapTimer::doWork(int id) {
    // 删除指定id结点，并触发回调函数
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    del_(i);
}

// 删除指定索引的定时器
void HeapTimer::del_(size_t index) {
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    // 将要删除的结点换到队尾，然后调整堆
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if(i < n) {
        SwapNode_(i, n);
        if(!siftdown_(i, n)) {
            siftup_(i);
        }
    }
    // 队尾元素删除
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

// 调整指定 id 的定时器过期时间
void HeapTimer::adjust(int id, int timeout) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);;
    siftdown_(ref_[id], heap_.size());
}

// 清除超时的定时器
void HeapTimer::tick() {
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();
        // 遇到没有超时的, 可以停止了, 前面超时的都被删了
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }
        node.cb();
        pop();
    }
}

// 删除堆顶的定时器
void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}

// 清空定时器堆
void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

// 获取下一个定时器距离超时的剩余时间
int HeapTimer::GetNextTick() {
    tick();                             // 清除超时的定时器
    size_t res = -1;
    if(!heap_.empty()) {
        // 计算下一个定时器距离超时的剩余时间
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}