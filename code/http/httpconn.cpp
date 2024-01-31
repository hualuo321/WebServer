#include "httpconn.h"
using namespace std;

const char* HttpConn::srcDir;                               // 资源路径
std::atomic<int> HttpConn::userCount;                       // 用户数量
bool HttpConn::isET;                                        // 是否边缘触发

// 构造函数
HttpConn::HttpConn() { 
    fd_ = -1;                                               // 文件描述符初始化为-1
    addr_ = { 0 };                                          // 地址初始化
    isClose_ = true;                                        // 连接是否关闭
};

// 析构函数
HttpConn::~HttpConn() { 
    Close(); 
};

// 初始化客户端连接对象
void HttpConn::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;                                            // 用户计数增加
    addr_ = addr;                                           // 设置地址信息
    fd_ = fd;                                               // 设置 fd
    writeBuff_.RetrieveAll();                               // 清空写缓冲区
    readBuff_.RetrieveAll();                                // 清空读缓冲区
    isClose_ = false;                                       // 设置客户端为开启状态
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

// 关闭连接
void HttpConn::Close() {
    response_.UnmapFile();                                  // 解除文件映射
    if(isClose_ == false){
        isClose_ = true;                                    // 设置为关闭状态
        userCount--;                                        // 用户计数减少
        close(fd_);                                         // 关闭文件描述符
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

// 获取文件描述符
int HttpConn::GetFd() const {
    return fd_;
};

// 获取客户端地址
struct sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

// 获取客户端IP
const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

// 获取客户端端口
int HttpConn::GetPort() const {
    return addr_.sin_port;
}

// 读取数据, 将客户端读缓冲区的内容
ssize_t HttpConn::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);             // 从文件描述符读取数据到缓冲区
        if (len <= 0) {
            break;
        }
    } while (isET);                                         // 如果是ET模式，需要循环读取直到没有更多数据
    return len;
}

// 写入数据
ssize_t HttpConn::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);                   // 使用writev写入数据
        if(len <= 0) {
            *saveErrno = errno;                             // 设置错误号
            break;
        }
        // 更新iov结构体，准备下一次写入
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; }
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuff_.Retrieve(len);
        }
    } while(isET || ToWriteBytes() > 10240);                // 如果是ET模式或者还有较多数据待发送，继续循环
    return len;
}

// 处理HTTP请求并生成响应
bool HttpConn::process() {
    request_.Init();                                        // 初始化HTTP请求对象
    if(readBuff_.ReadableBytes() <= 0) {                    // 如果可读字节小于等于0，则返回false
        return false;
    }
    else if(request_.parse(readBuff_)) {                    // 解析请求
        LOG_DEBUG("%s", request_.path().c_str());           // 记录调试信息
        // 初始化HTTP响应对象，设置为200 OK
        response_.Init(srcDir, request_.path(), request_.IsKeepAlive(), 200);
    } else {
        // 解析失败，初始化HTTP响应对象，设置为400 Bad Request
        response_.Init(srcDir, request_.path(), false, 400);
    }
    // 生成响应内容
    response_.MakeResponse(writeBuff_);
    // 设置响应头的iovec结构
    iov_[0].iov_base = const_cast<char*>(writeBuff_.Peek());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;                                            // iovec计数

    // 如果有文件内容要发送，设置文件内容的iovec结构
    if(response_.FileLen() > 0  && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;                                        // iovec计数为2，表示有文件内容
    }   
    LOG_DEBUG("filesize:%d, %d  to %d", response_.FileLen() , iovCnt_, ToWriteBytes());
    return true;                                            // 返回true表示处理成功
}