#include "webserver.h"
using namespace std;

// 构造函数
WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, int sqlPort, const char* sqlUser,  const  char* sqlPwd, 
                    const char* dbName, int connPoolNum, int threadNum, bool openLog,  int logLevel, int logQueSize): 
                    port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS),  isClose_(false), timer_(new HeapTimer()),  
                    threadpool_(new ThreadPool(threadNum)),  epoller_(new Epoller()) {
    srcDir_ = getcwd(nullptr, 256);         // 获取静态资源地址
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16); 
    HttpConn::userCount = 0;                // 初始化用户连接数
    HttpConn::srcDir = srcDir_;             // 初始化用户静态资源地址
    // 初始化连接池单例
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
    // 初始化事件模式
    InitEventMode_(trigMode);
    // 初始化服务器监听 socket
    if(!InitSocket_()) { isClose_ = true;}                          
    // 初始化日志单例
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

// 析构函数
WebServer::~WebServer() {
    close(listenFd_);                                               // 关闭监听文件描述符
    isClose_ = true;                                                // 标记服务器关闭
    free(srcDir_);                                                  // 释放资源目录字符串
    SqlConnPool::Instance()->ClosePool();                           // 关闭SQL连接池
}

// 初始化事件模式
void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;              // 默认监听事件 (读关闭)
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP; // 默认连接事件, ONESHOT 代表一个线程只能处理一个连接
    // 根据输入选择模式 (这些模式判断是通过位操作实现的, | 就是相应位置置 1, & 就是判断相应位置是否为 1)
    switch (trigMode)
    {
    case 0:
        break;                              // 保持默认
    case 1:
        connEvent_ |= EPOLLET;              // 只对连接事件使用边缘触发 (ET)
        break;
    case 2:
        listenEvent_ |= EPOLLET;            // 只对监听事件使用边缘触发 (ET)
        break;
    case 3:
        listenEvent_ |= EPOLLET;            // 监听和连接事件都使用边缘触发 (ET)
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;            // 默认设置为边缘触发
        connEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);// 设置客户端是否为边缘触发 (ET)
}

/*  启动 Web 服务器
    - 现在有一个定时器, 里面装的是所有 fd 的过期剩余时间, 用小根堆来存储, 根节点代表距离过期事件最短。
    - 于是 epoll 实例获取那个最快的过期时间(正数), 并将之前已过期的时间清除(负数)。如果在这期间有事件发生, 就依次进行处理。
    - epoll 会先从事件队列中取出一个事件, 可能是连接事件, 读事件, 写事件, 错误事件, 并进行相应的处理。
 */
void WebServer::Start() {
    int timeMS = -1;                                    // 下一个定时器距离超时的剩余时间
    if(!isClose_) { LOG_INFO("========== Server start =========="); }
    while(!isClose_) {                                  // 服务器没有关闭, 则一直运行
        if(timeoutMS_ > 0) {                            // 超时时间初始化为 6000ms
            timeMS = timer_->GetNextTick();             // 获取下一个超时时间
        }
        int eventCnt = epoller_->Wait(timeMS);          // 等待事件发生, 最多阻塞 timeMS, 因为此时已经有事件过期, 把过期 fd 给断开连接
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
    epoller_->DelFd(client->GetFd());               // 从 epoll 实例中删除 fd
    client->Close();                                // 关闭 Http 连接对象
}

// 添加新客户端
void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);                      // 初始化 Http 连接对象
    if(timeoutMS_ > 0) {
        // 如果设置了超时时间，将超时事件及对应的 fd 添加到定时器中
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);      // 将 fd 添加到 epoll 实例, 监听读事件
    SetFdNonblock(fd);                              // 设置 fd 为非阻塞模式
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

/*  处理监听事件
    - 先创建一个空 c_addr 用于存放客户端的地址信息, 后续通过 accept() 将地址信息写入 c_addr
    - 根据监听到的 cfd, c_addr 来添加 cfd 到 epoll 中, 并更新定时器,等信息
    - 设置 cfd 为 ET 模式, 处理过的连接事件只会通知一次 
*/
void WebServer::DealListen_() {
    struct sockaddr_in addr;                        // 用于存放客户端的地址信息
    socklen_t len = sizeof(addr);
    do {
        // 将 accept 的客户信息放入 addr 中, 并返回 fd
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len); 
        if(fd <= 0) { return;}
        else if(HttpConn::userCount >= MAX_FD) {    // 如果当前用户数超过最大值，则发送服务器忙的消息并返回
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient_(fd, addr);                       // 添加新客户端
    } while(listenEvent_ & EPOLLET);                // 如果是边缘触发，则循环接受所有连接
}

// 处理读事件
void WebServer::DealRead_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);                            // 如果发生读写事件, 则更新定时器, 重置超时时间
    // 将读任务添加到线程池, 等待工作线程进行处理
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

// 处理写事件
void WebServer::DealWrite_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);                            // 如果发生读写事件, 则更新定时器, 重置超时时间
    // 将写任务添加到线程池, 等待工作线程进行处理
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

// 延长客户端的超时时间
void WebServer::ExtentTime_(HttpConn* client) {
    assert(client);
    // 调整客户端在定时器中的时间
    if(timeoutMS_ > 0) { timer_->adjust(client->GetFd(), timeoutMS_); }
}

// 读取数据
void WebServer::OnRead_(HttpConn* client) {
    assert(client);
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

// 初始化服务端的 socket
bool WebServer::InitSocket_() {
    int ret;
    struct sockaddr_in addr;                // 存放服务端 socket 的地址信息
    if(port_ > 65535 || port_ < 1024) {     // 检查服务器端口号是否有效
        LOG_ERROR("Port:%d error!",  port_);
        return false;
    }
    // 设置服务端地址信息 (协议族类型, IP 地址, 端口号)
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    // 设置 是否优雅关闭
    struct linger optLinger = { 0 };
    if(openLinger_) {
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    // 创建监听 socket, 用于监听连接
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }
    // 设置套接字选项, 这里设置的是优雅关闭
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }
    // 设置套接字选项, 这里设置的是端口复用
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }
    // 将服务端的 Socket 地址信息绑定到 listenFd 上
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    // 将 listenFd 切换为监听状态
    ret = listen(listenFd_, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    // 将 listenFd 添加到 epoll 实例 (可读事件)
    ret = epoller_->AddFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    // 设置 listenFd 为非阻塞模式
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