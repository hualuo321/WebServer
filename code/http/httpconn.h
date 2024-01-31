#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>                                // readv/writev
#include <arpa/inet.h>                              // sockaddr_in
#include <stdlib.h>                                 // atoi()
#include <errno.h>      

#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

// 客户端对应的连接类
class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr); // 连接初始化
    ssize_t read(int* saveErrno);                   // 读取数据
    ssize_t write(int* saveErrno);                  // 写入数据
    void Close();                                   // 关闭连接
    int GetFd() const;                              // 获取 fd
    int GetPort() const;                            // 获取 port
    const char* GetIP() const;                      // 获取 IP
    sockaddr_in GetAddr() const;                    // 获取 地址信息
    bool process();                                 // 处理用户请求, 比如解析 HTTP 网页

    int ToWriteBytes() {                            // 待写入数据的字节数
        return iov_[0].iov_len + iov_[1].iov_len;   
    }
    bool IsKeepAlive() const {                      // 判断是否保持连接
        return request_.IsKeepAlive();
    }

    static bool isET;                               // 是否使用边缘触发
    static const char* srcDir;                      // 静态资源目录
    static std::atomic<int> userCount;              // 客户端连接数

private:
    int fd_;                                        // 文件描述符
    struct  sockaddr_in addr_;                      // 客户端地址信息 (协议族, TCP, IP 端口)

    bool isClose_;                                  // 连接是否关闭
    
    int iovCnt_;                                    // 分散读写的通道数
    struct iovec iov_[2];                           // iovec 数组，用于分散读写
    
    Buffer readBuff_;                               // 读缓冲区
    Buffer writeBuff_;                              // 写缓冲区

    HttpRequest request_;                           // HTTP请求对象
    HttpResponse response_;                         // HTTP响应对象
};

#endif //HTTP_CONN_H