#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

// ģ���� BlockDeque, ����ʵ��һ������˫�˶���
template<class T>
class BlockDeque {
public:
    // ���캯����ָ�����������Ĭ��Ϊ1000
    explicit BlockDeque(size_t MaxCapacity = 1000);
    // ��������
    ~BlockDeque();
    // ��ն���
    void clear();
    // �ж϶����Ƿ�Ϊ��
    bool empty();
    // �ж϶����Ƿ�����
    bool full();
    // �رն���
    void Close();
    // ��ȡ���е�ǰ��С
    size_t size();
    // ��ȡ�����������
    size_t capacity();
    // ��ȡ����ͷ��Ԫ��
    T front();
    // ��ȡ����β��Ԫ��
    T back();
    // �ڶ���β�����Ԫ��
    void push_back(const T &item);
    // �ڶ���ͷ�����Ԫ��
    void push_front(const T &item);
    // �Ӷ���ͷ���Ƴ�Ԫ�أ��������Ϊ����ȴ�
    bool pop(T &item);
    // �Ӷ���ͷ���Ƴ�Ԫ�أ��������Ϊ����ȴ�һ��ʱ��
    bool pop(T &item, int timeout);
    // ֪ͨ�����������ݿɶ�
    void flush();

private:
    // STL˫�˶��У����ڴ洢Ԫ��
    std::deque<T> deq_;
    // ���е��������
    size_t capacity_;
    // �����������ڱ�������
    std::mutex mtx_;
    // ��ʾ�����Ƿ񱻹ر�
    bool isClose_;
    // ��������������
    std::condition_variable condConsumer_;
    // ��������������
    std::condition_variable condProducer_;
};

// ���캯������ʼ�����е��������
template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) :capacity_(MaxCapacity) {
    assert(MaxCapacity > 0);                        // ������������������0
    isClose_ = false;                               // ��ʼ������Ϊ����״̬
}

// �����������رն���
template<class T>
BlockDeque<T>::~BlockDeque() {
    Close();
};

// �رն���
template<class T>
void BlockDeque<T>::Close() {
    {   
        std::lock_guard<std::mutex> locker(mtx_);   // ��������
        deq_.clear();                               // ��ն���
        isClose_ = true;                            // ���ö���Ϊ�ر�״̬
    }
    condProducer_.notify_all();                     // �������еȴ���������
    condConsumer_.notify_all();                     // �������еȴ���������
};

// ֪ͨ�����������ݿɶ�
template<class T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_one();                     // ����һ���ȴ���������
};

// ��ն���
template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);       // ��������
    deq_.clear();                                   // ��ն���
}

// ��ȡ����ͷ��Ԫ��
template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);       // ��������
    return deq_.front();                            // ���ض���ͷ��Ԫ��
}

// ��ȡ����β��Ԫ��
template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);       // ��������
    return deq_.back();                             // ���ض���β��Ԫ��
}

// ��ȡ���е�ǰ��С
template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);       // ��������
    return deq_.size();                             // ���ض��д�С
}

// ��ȡ�����������
template<class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);       // ��������
    return capacity_;                               // ���ض����������
}

// �ڶ���β�����һ��Ԫ��
template<class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);      // ��������
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker);                 // ���������������ȴ�
    }
    deq_.push_back(item);                           // �����β�����Ԫ��
    condConsumer_.notify_one();                     // ֪ͨһ���ȴ���������
}

// �ڶ���ͷ�����һ��Ԫ��
template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);      // ��������
    while(deq_.size() >= capacity_) {               
        condProducer_.wait(locker);                 // ���������������ȴ�
    }
    deq_.push_front(item);                          // �����ͷ�����Ԫ��
    condConsumer_.notify_one();                     // ֪ͨһ���ȴ���������
}

// �ж϶����Ƿ�Ϊ��
template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);       // ��������
    return deq_.empty();                            // ���ض����Ƿ�Ϊ��
}

// �ж϶����Ƿ�����
template<class T>
bool BlockDeque<T>::full(){
    std::lock_guard<std::mutex> locker(mtx_);       // ��������
    return deq_.size() >= capacity_;                // ���ض����Ƿ�����
}

// �Ӷ���ͷ���Ƴ�һ��Ԫ��
template<class T>
bool BlockDeque<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);      // ��������
    while(deq_.empty()){
        condConsumer_.wait(locker);                 // �������Ϊ�գ���ȴ�
        if(isClose_){
            return false;                           // ������йرգ��򷵻�false
        }
    }
    item = deq_.front();                            // ��ȡ����ͷ��Ԫ��
    deq_.pop_front();                               // �Ӷ���ͷ���Ƴ�Ԫ��
    condProducer_.notify_one();                     // ֪ͨһ���ȴ���������
    return true;
}

// �Ӷ���ͷ���Ƴ�һ��Ԫ�أ�����ʱ
template<class T>
bool BlockDeque<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);      // ��������
    while(deq_.empty()){                            // �������Ϊ�գ���ȴ�
        if(condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout){
            return false;                           // �ȴ�ָ���ĳ�ʱʱ��, ��ʱ����false
        }
        if(isClose_){
            return false;                           // ������йرգ��򷵻�false
        }
    }
    item = deq_.front();                            // ��ȡ����ͷ��Ԫ��
    deq_.pop_front();                               // �Ӷ���ͷ���Ƴ�Ԫ��
    condProducer_.notify_one();                     // ֪ͨһ���ȴ���������
    return true;
}

#endif // BLOCKQUEUE_H