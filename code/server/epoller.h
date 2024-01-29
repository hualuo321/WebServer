#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>                              // epoll_ctl()
#include <fcntl.h>                                  // fcntl()
#include <unistd.h>                                 // close()
#include <assert.h>                                 // close()
#include <vector>
#include <errno.h>

// Epoller类，封装了epoll的操作
class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);          // 构造函数，显式声明避免隐式类型转换
    ~Epoller();                                     // 析构函数
    bool AddFd(int fd, uint32_t events);            // 添加文件描述符到epoll实例
    bool ModFd(int fd, uint32_t events);            // 修改epoll实例中的文件描述符
    bool DelFd(int fd);                             // 从epoll实例中删除文件描述符
    int Wait(int timeoutMs = -1);                   // 等待事件发生，timeoutMs指定超时时间，单位是毫秒
    int GetEventFd(size_t i) const;                 // 获取第i个事件的文件描述符
    uint32_t GetEvents(size_t i) const;             // 获取第i个事件的类型

private:
    int epollFd_;                                   // epoll文件描述符
    std::vector<struct epoll_event> events_;        // 用于存储epoll事件的集合
};

#endif //EPOLLER_H