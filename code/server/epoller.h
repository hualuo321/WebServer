#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

// Epoller 类，封装了 epoll 的操作
class Epoller {
public:
    // 构造函数，显式声明避免隐式类型转换
    explicit Epoller(int maxEvent = 1024);
    // 析构函数
    ~Epoller();
    // 添加 fd 到 epoll 实例中
    bool AddFd(int fd, uint32_t events);
    // 修改 fd 到 epoll 实例中
    bool ModFd(int fd, uint32_t events);
    // 删除 fd 到 epoll 实例中
    bool DelFd(int fd);
    // 等待事件发生，timeoutMs 指定超时时间
    int Wait(int timeoutMs = -1);
    // 获取第 i 个事件的 fd
    int GetEventFd(size_t i) const;
    // 获取第 i 个事件的类型
    uint32_t GetEvents(size_t i) const;

private:
    int epollFd_;                                   // epoll 实例的 fd
    std::vector<struct epoll_event> events_;        // 用于存储监听 fd 事件类型的数组
};

#endif //EPOLLER_H