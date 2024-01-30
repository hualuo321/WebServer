# git 使用
```git
git status 														// 查询修改状态
git add .														// 想要提交所有的文件
git commit -m "update"											// 为提交的代码添加提交注释
git push -u origing master										// 将代码提交到主分支
```

# 常用函数
```cpp
# socket: 创建一个新的套接字 (协议族, 套接字类型, 是否为默认协议) (套接字 fd)
int socket(int domain, int type, int protocol);
int sockfd = socket(AF_INET, SOCK_STREAM, 0);

# setsockopt: 设置套接字选项 (sockfd, 代码级别, 要修改的选项, 缓冲区指针, 缓冲区大小) (操作是否成功)
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
```

# 常用结构
```cpp
# 存放 socket 中的信息
struct sockaddr_in {
	short	sin_family;                                         // 协议族, 比如 IPv4(AF_INET)
	u_short	sin_port;                                           // 端口号
	struct in_addr	sin_addr;                                   // IP 地址
	char	sin_zero[8];                                        // 填充字段
};

# epoll 中的事件, 包含事件类型(位掩码实现), 和事件相关的额外信息
struct epoll_event {
    uint32_t     events;                                        // 可读(EPOLLIN), 可写(EPOLLOUT), 挂起(EPOLLHUP), ET(EPOLLET)
    epoll_data_t data;                                          // 包含 fd, 指针, int32, int64
};
```