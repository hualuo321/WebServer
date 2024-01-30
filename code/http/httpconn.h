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

// HttpConn 类，处理一个HTTP连接
class HttpConn {
public:
    HttpConn();                                     // 构造函数
    ~HttpConn();                                    // 析构函数

    void init(int sockFd, const sockaddr_in& addr); // 初始化连接
    ssize_t read(int* saveErrno);                   // 读取数据
    ssize_t write(int* saveErrno);                  // 写入数据
    void Close();                                   // 关闭连接
    int GetFd() const;                              // 获取文件描述符
    int GetPort() const;                            // 获取端口
    const char* GetIP() const;                      // 获取IP地址
    sockaddr_in GetAddr() const;                    // 获取sockaddr_in地址
    bool process();                                 // 处理读取到的请求

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
    struct  sockaddr_in addr_;                      // 客户端地址

    bool isClose_;                                  // 连接是否关闭
    
    int iovCnt_;                                    // writev使用的iovec计数
    struct iovec iov_[2];                           // iovec数组，用于readv/writev
    
    Buffer readBuff_;                               // 读缓冲区
    Buffer writeBuff_;                              // 写缓冲区

    HttpRequest request_;                           // HTTP请求
    HttpResponse response_;                         // HTTP响应
};

#endif //HTTP_CONN_H