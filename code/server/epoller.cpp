#include "epoller.h"

// Epoller 类的构造函数
Epoller::Epoller(int maxEvent):epollFd_(epoll_create(512)), events_(maxEvent){
    // 确保epoll文件描述符创建成功，且预分配的事件向量大小大于0
    assert(epollFd_ >= 0 && events_.size() > 0);
}

// Epoller 类的析构函数
Epoller::~Epoller() {
    close(epollFd_);                                            // 关闭epoll文件描述符
}

// 添加文件描述符到epoll实例
bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;                                    // 检查文件描述符是否有效
    epoll_event ev = {0};                                       // 创建epoll_event结构体
    ev.data.fd = fd;                                            // 设置文件描述符
    ev.events = events;                                         // 设置事件类型
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);    // 向epoll实例添加文件描述符
}

// 修改epoll实例中的文件描述符
bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;                                    // 检查文件描述符是否有效
    epoll_event ev = {0};                                       // 创建epoll_event结构体
    ev.data.fd = fd;                                            // 设置文件描述符
    ev.events = events;                                         // 设置事件类型
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);    // 修改epoll实例中的文件描述符
}

// 从epoll实例中删除文件描述符
bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;                                    // 检查文件描述符是否有效
    epoll_event ev = {0};                                       // 创建epoll_event结构体
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);    // 从epoll实例中删除文件描述符
}

// 等待事件发生
int Epoller::Wait(int timeoutMs) {
    // epoll_wait等待事件发生，返回发生的事件数量
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

// 获取第i个事件的文件描述符
int Epoller::GetEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);                       // 确保索引i有效
    return events_[i].data.fd;                                  // 返回第i个事件的文件描述符
}

// 获取第i个事件的类型
uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);                       // 确保索引i有效
    return events_[i].events;                                   // 返回第i个事件的类型
}