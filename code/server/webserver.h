#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

// 服务器类
class WebServer {
public:
    // 构造函数
    WebServer(int port, int trigMode, int timeoutMS, bool OptLinger,  int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum, bool openLog, int logLevel, int logQueSize);
    ~WebServer();
    void Start();                                   // 启动服务器

private:
    bool InitSocket_();                             // 初始化 socket
    void InitEventMode_(int trigMode);              // 初始化事件模式
    void AddClient_(int fd, sockaddr_in addr);      // 添加客户端
    void DealListen_();                             // 处理监听事件
    void DealWrite_(HttpConn* client);              // 处理写事件
    void DealRead_(HttpConn* client);               // 处理读事件
    void SendError_(int fd, const char*info);       // 发送错误信息
    void ExtentTime_(HttpConn* client);             // 延长客户端时间
    void CloseConn_(HttpConn* client);              // 关闭连接
    void OnRead_(HttpConn* client);                 // 读取数据处理
    void OnWrite_(HttpConn* client);                // 写入数据处理
    void OnProcess(HttpConn* client);               // 处理请求

    static const int MAX_FD = 65536;                // 最大 fd 数量
    static int SetFdNonblock(int fd);               // 设置 fd 为非阻塞模式

    int port_;                                      // 服务器端口
    bool openLinger_;                               // 是否开启优雅关闭
    int timeoutMS_;                                 // 超时时间（毫秒）
    bool isClose_;                                  // 是否关闭服务器
    int listenFd_;                                  // 监听 fd 
    char* srcDir_;                                  // 静态资源目录
    
    uint32_t listenEvent_;                          // 监听事件
    uint32_t connEvent_;                            // 连接事件
   
    std::unique_ptr<HeapTimer> timer_;              // 小根堆定时器
    std::unique_ptr<ThreadPool> threadpool_;        // 线程池
    std::unique_ptr<Epoller> epoller_;              // Epoll实例
    std::unordered_map<int, HttpConn> users_;       // 存储客户端连接用户
};

#endif //WEBSERVER_H