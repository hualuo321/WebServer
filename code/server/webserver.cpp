#include "webserver.h"
using namespace std;

// WebServer 类的构造函数
WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum, bool openLog, int logLevel, int logQueSize):
            port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false), timer_(new HeapTimer()), 
            threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller()) {
    srcDir_ = getcwd(nullptr, 256);                                 // 获取当前工作目录
    assert(srcDir_);                                                // 断言目录获取成功
    strncat(srcDir_, "/resources/", 16);                            // 连接资源目录
    HttpConn::userCount = 0;                                        // 初始化用户计数
    HttpConn::srcDir = srcDir_;                                     // 设置静态资源目录
    // 初始化SQL连接池
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
    InitEventMode_(trigMode);                                       // 初始化事件模式
    if(!InitSocket_()) { isClose_ = true;}                          // 初始化套接字
    // 如果打开日志记录
    if(openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose_) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent_ & EPOLLET ? "ET": "LT"),
                            (connEvent_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

// WebServer 类的析构函数
WebServer::~WebServer() {
    close(listenFd_);                                               // 关闭监听文件描述符
    isClose_ = true;                                                // 标记服务器关闭
    free(srcDir_);                                                  // 释放资源目录字符串
    SqlConnPool::Instance()->ClosePool();                           // 关闭SQL连接池
}

// 初始化事件模式
void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;                                      // 默认监听事件 (读关闭)
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;                         // 默认连接事件 (ET | 读关闭)
    switch (trigMode)
    {
    case 0:
        break;                                                      // 保持默认
    case 1:
        connEvent_ |= EPOLLET;                                      // 只对连接使用边缘触发(ET)
        break;
    case 2:
        listenEvent_ |= EPOLLET;                                    // 只对监听使用边缘触发(ET)
        break;
    case 3:
        listenEvent_ |= EPOLLET;                                    // 监听和连接都使用边缘触发(ET)
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;                                    // 默认设置为边缘触发
        connEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);                        // 设置Http连接是否使用ET模式
}

// 启动Web服务器
void WebServer::Start() {
    int timeMS = -1;                                                // epoll等待超时时间设置为-1，即无事件将阻塞
    if(!isClose_) { LOG_INFO("========== Server start =========="); }
    while(!isClose_) {                                              // 服务器没有关闭, 则一直运行
        if(timeoutMS_ > 0) {
            timeMS = timer_->GetNextTick();
        }
        int eventCnt = epoller_->Wait(timeMS);                      // 等待事件发生
        for(int i = 0; i < eventCnt; i++) {
            // 处理每个事件
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);               // 获取发生事件的 fd
            if(fd == listenFd_) {
                DealListen_();                                      // 如果是 lfd， 则处理连接事件
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {  // 处理连接断开、挂起、错误事件
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }
            else if(events & EPOLLIN) {                             // 处理可读事件
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            else if(events & EPOLLOUT) {                            // 处理可写事件
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event");                      // 处理未预期事件
            }
        }
    }
}

// 向客户端发送错误信息
void WebServer::SendError_(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);                      // 发送错误信息
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);                                                      // 关闭文件描述符
}

// 关闭连接
void WebServer::CloseConn_(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());                               // 从epoller中删除文件描述符
    client->Close();                                                // 关闭Http连接
}

// 添加新客户端
void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);                                      // 初始化Http连接
    if(timeoutMS_ > 0) {
        // 如果设置了超时时间，添加到定时器中
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);                      // 将文件描述符添加到epoller, 监听读事件
    SetFdNonblock(fd);                                              // 设置文件描述符为非阻塞模式
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

// 处理监听事件
void WebServer::DealListen_() {
    struct sockaddr_in addr;                                        // 用于存放客户端的 IP 端口
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len); // 将 accept 的客户信息放入 addr 中, 并返回 fd
        if(fd <= 0) { return;}                                      // 如果文件描述符小于或等于0，则失败返回
        else if(HttpConn::userCount >= MAX_FD) {                    // 如果当前用户数超过最大值，则发送服务器忙的消息并返回
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient_(fd, addr);                                       // 添加新客户端
    } while(listenEvent_ & EPOLLET);                                // 如果是边缘触发，则循环接受所有连接
}

// 处理读事件
void WebServer::DealRead_(HttpConn* client) {
    assert(client);                                                 // 断言客户端不为空
    ExtentTime_(client);                                            // 延长客户端的超时时间
    // 将读任务添加到线程池
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

// 处理写事件
void WebServer::DealWrite_(HttpConn* client) {
    assert(client);                                                 // 断言客户端不为空
    ExtentTime_(client);                                            // 延长客户端的超时时间
    // 将写任务添加到线程池
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

// 延长客户端的超时时间
void WebServer::ExtentTime_(HttpConn* client) {
    assert(client);                                                 // 断言客户端不为空
    // 调整客户端在定时器中的时间
    if(timeoutMS_ > 0) { timer_->adjust(client->GetFd(), timeoutMS_); }
}

// 读取数据
void WebServer::OnRead_(HttpConn* client) {
    assert(client);                                                 // 断言客户端不为空
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);                                 // 读取数据
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);                                         // 如果读取出错并且错误不是EAGAIN，则关闭连接
        return;
    }
    OnProcess(client);                                              // 处理读取到的数据 (解析 HTTP 请求)
}

// 处理客户端请求
void WebServer::OnProcess(HttpConn* client) {
    if(client->process()) {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);    // 如果处理成功，则注册写事件
    } else {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);     // 否则注册读事件
    }
}

// 写入数据
void WebServer::OnWrite_(HttpConn* client) {
    assert(client);                                                 // 断言客户端不为空
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);                               // 写入数据
    if(client->ToWriteBytes() == 0) {                               // 如果数据传输完成
        if(client->IsKeepAlive()) {                                 // 如果保持连接，则继续处理客户端请求
            OnProcess(client);                                      
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {                                  // 如果写入出错并且错误是EAGAIN，则继续写入数据 (ET)
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);                                             // 关闭连接
}

// 初始化套接字
bool WebServer::InitSocket_() {
    int ret;
    struct sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024) {                             // 检查端口号是否有效
        LOG_ERROR("Port:%d error!",  port_);
        return false;
    }
    // 设置地址结构
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    // 设置 linger 选项以控制连接关闭的方式
    struct linger optLinger = { 0 };
    if(openLinger_) {                                               
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    // 创建监听套接字
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }
    // 设置套接字选项
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }
    // 设置端口复用, 只有最后一个套接字会正常接收数据。
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }
    // 绑定地址和端口
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    // 开始监听
    ret = listen(listenFd_, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    // 添加监听套接字到 epoll 实例
    ret = epoller_->AddFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    // 设置监听文件描述符为非阻塞模式
    SetFdNonblock(listenFd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}

// 设置 fd 为非阻塞
int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    // 获取 fd 原来的 FL, 并修改为非阻塞, 设置为新的 FL
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}


