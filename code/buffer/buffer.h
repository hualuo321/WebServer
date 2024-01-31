#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>                              // 用于使用 perror 函数
#include <iostream>
#include <unistd.h>                             // 用于使用 write 函数
#include <sys/uio.h>                            // 用于使用 readv 函数
#include <vector>                      
#include <atomic>
#include <assert.h>

// 缓冲区类 定义
class Buffer {
public:
    // 构造函数，初始化缓冲区大小，默认为1024字节
    Buffer(int initBuffSize = 1024);
    // 默认析构函数
    ~Buffer() = default;
    // 返回可写的字节数
    size_t WritableBytes() const;
    // 返回可读的字节数
    size_t ReadableBytes() const ;
    // 返回预备区的字节数
    size_t PrependableBytes() const;
    // 返回缓冲区中可读数据的指针
    const char* Peek() const;
    // 确保缓冲区有足够的可写空间
    void EnsureWriteable(size_t len);
    // 更新缓冲区已写入的字节数
    void HasWritten(size_t len);
    // 读取指定长度的数据
    void Retrieve(size_t len);
    // 读取直到指定位置的数据
    void RetrieveUntil(const char* end);
    // 清空缓冲区
    void RetrieveAll() ;
    // 读取全部数据并返回为字符串
    std::string RetrieveAllToStr();
    // 获取可写数据起始位置的指针（非const版本）
    char* BeginWrite();
    // 获取可写数据起始位置的指针（const版本）
    const char* BeginWriteConst() const;
    // 追加字符串到缓冲区
    void Append(const std::string& str);
    // 追加指定长度的字符串到缓冲区
    void Append(const char* str, size_t len);
    // 追加指定长度的二进制数据到缓冲区
    void Append(const void* data, size_t len);
    // 追加另一个缓冲区的数据到当前缓冲区
    void Append(const Buffer& buff);
    // 从 fd 中读取数据到缓冲区
    ssize_t ReadFd(int fd, int* Errno);
    // 将缓冲区的数据写入文件描述符
    ssize_t WriteFd(int fd, int* Errno);

private:
    // 获取缓冲区数据起始位置的指针（非const版本）
    char* BeginPtr_();
    // 获取缓冲区数据起始位置的指针（const版本）
    const char* BeginPtr_() const;
    // 确保缓冲区有足够的空间
    void MakeSpace_(size_t len);
    // 缓冲区数据存储的容器
    std::vector<char> buffer_;
    // 读取位置的原子变量
    std::atomic<std::size_t> readPos_;
    // 写入位置的原子变量
    std::atomic<std::size_t> writePos_;
};

#endif //BUFFER_H