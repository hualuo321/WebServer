#include "epoller.h"

// 构造函数 (调用 epoll_create 创建一个 epoll 实例)
Epoller::Epoller(int maxEvent):epollFd_(epoll_create(512)), events_(maxEvent){
    assert(epollFd_ >= 0 && events_.size() > 0);
}

// 析构函数
Epoller::~Epoller() {
    close(epollFd_);
}

// 添加 fd 到 epoll 实例中
bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};                                       // 创建 epoll_event 结构体
    ev.data.fd = fd;                                            // 设置文件描述符
    ev.events = events;                                         // 设置事件类型
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);    // 向 epoll 实例添加 fd
}

// 修改 fd 到 epoll 实例中
bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};                                       // 创建 epoll_event 结构体
    ev.data.fd = fd;                                            // 设置文件描述符
    ev.events = events;                                         // 设置事件类型
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);    // 向 epoll 实例修改 fd
}

// 删除 fd 到 epoll 实例中
bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};                                       // 创建 epoll_event 结构体
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);    // 向 epoll 实例删除 fd
}

// 等待事件发生, 并返回发生的事件数量
int Epoller::Wait(int timeoutMs) {
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

// 获取第 i 个事件的 fd
int Epoller::GetEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

// 获取第 i 个事件的类型
uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}