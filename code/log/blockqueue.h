#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

// 模板类 BlockDeque, 用于实现一个阻塞双端队列
template<class T>
class BlockDeque {
public:
    // 构造函数，指定最大容量，默认为1000
    explicit BlockDeque(size_t MaxCapacity = 1000);
    // 析构函数
    ~BlockDeque();
    // 清空队列
    void clear();
    // 判断队列是否为空
    bool empty();
    // 判断队列是否已满
    bool full();
    // 关闭队列
    void Close();
    // 获取队列当前大小
    size_t size();
    // 获取队列最大容量
    size_t capacity();
    // 获取队列头部元素
    T front();
    // 获取队列尾部元素
    T back();
    // 在队列尾部添加元素
    void push_back(const T &item);
    // 在队列头部添加元素
    void push_front(const T &item);
    // 从队列头部移除元素，如果队列为空则等待
    bool pop(T &item);
    // 从队列头部移除元素，如果队列为空则等待一定时间
    bool pop(T &item, int timeout);
    // 通知消费者有数据可读
    void flush();

private:
    // STL双端队列，用于存储元素
    std::deque<T> deq_;
    // 队列的最大容量
    size_t capacity_;
    // 互斥量，用于保护队列
    std::mutex mtx_;
    // 表示队列是否被关闭
    bool isClose_;
    // 消费者条件变量
    std::condition_variable condConsumer_;
    // 生产者条件变量
    std::condition_variable condProducer_;
};

// 构造函数，初始化队列的最大容量
template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) :capacity_(MaxCapacity) {
    assert(MaxCapacity > 0);                        // 断言最大容量必须大于0
    isClose_ = false;                               // 初始化队列为开启状态
}

// 析构函数，关闭队列
template<class T>
BlockDeque<T>::~BlockDeque() {
    Close();
};

// 关闭队列
template<class T>
void BlockDeque<T>::Close() {
    {   
        std::lock_guard<std::mutex> locker(mtx_);   // 加锁保护
        deq_.clear();                               // 清空队列
        isClose_ = true;                            // 设置队列为关闭状态
    }
    condProducer_.notify_all();                     // 唤醒所有等待的生产者
    condConsumer_.notify_all();                     // 唤醒所有等待的消费者
};

// 通知消费者有数据可读
template<class T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_one();                     // 唤醒一个等待的消费者
};

// 清空队列
template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);       // 加锁保护
    deq_.clear();                                   // 清空队列
}

// 获取队列头部元素
template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);       // 加锁保护
    return deq_.front();                            // 返回队列头部元素
}

// 获取队列尾部元素
template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);       // 加锁保护
    return deq_.back();                             // 返回队列尾部元素
}

// 获取队列当前大小
template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);       // 加锁保护
    return deq_.size();                             // 返回队列大小
}

// 获取队列最大容量
template<class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);       // 加锁保护
    return capacity_;                               // 返回队列最大容量
}

// 在队列尾部添加一个元素
template<class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);      // 加锁保护
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker);                 // 如果队列已满，则等待
    }
    deq_.push_back(item);                           // 向队列尾部添加元素
    condConsumer_.notify_one();                     // 通知一个等待的消费者
}

// 在队列头部添加一个元素
template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);      // 加锁保护
    while(deq_.size() >= capacity_) {               
        condProducer_.wait(locker);                 // 如果队列已满，则等待
    }
    deq_.push_front(item);                          // 向队列头部添加元素
    condConsumer_.notify_one();                     // 通知一个等待的消费者
}

// 判断队列是否为空
template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);       // 加锁保护
    return deq_.empty();                            // 返回队列是否为空
}

// 判断队列是否已满
template<class T>
bool BlockDeque<T>::full(){
    std::lock_guard<std::mutex> locker(mtx_);       // 加锁保护
    return deq_.size() >= capacity_;                // 返回队列是否已满
}

// 从队列头部移除一个元素
template<class T>
bool BlockDeque<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);      // 加锁保护
    while(deq_.empty()){
        condConsumer_.wait(locker);                 // 如果队列为空，则等待
        if(isClose_){
            return false;                           // 如果队列关闭，则返回false
        }
    }
    item = deq_.front();                            // 获取队列头部元素
    deq_.pop_front();                               // 从队列头部移除元素
    condProducer_.notify_one();                     // 通知一个等待的生产者
    return true;
}

// 从队列头部移除一个元素，带超时
template<class T>
bool BlockDeque<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);      // 加锁保护
    while(deq_.empty()){                            // 如果队列为空，则等待
        if(condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout){
            return false;                           // 等待指定的超时时间, 超时返回false
        }
        if(isClose_){
            return false;                           // 如果队列关闭，则返回false
        }
    }
    item = deq_.front();                            // 获取队列头部元素
    deq_.pop_front();                               // 从队列头部移除元素
    condProducer_.notify_one();                     // 通知一个等待的生产者
    return true;
}

#endif // BLOCKQUEUE_H