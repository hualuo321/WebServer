#include "buffer.h"

// 构造函数，初始化缓冲区和读写位置
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

// 返回剩余可读字节数
size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}

// 返回可写字节数
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

// 表示已经读取的数据大小
size_t Buffer::PrependableBytes() const {
    return readPos_;
}

// 返回缓冲区中可读数据的指针
const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

// 从缓冲区中取出指定长度的数据, 并将读指针向前移动
void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}

// 从缓冲区中取出数据，直到指定的结束位置
void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

// 清空缓冲区，将缓冲区的内容全部重置为初始状态，以便重新使用
void Buffer::RetrieveAll() {
    // 使用 bzero 函数将缓冲区的内容全部置为零，相当于清空缓冲区
    bzero(&buffer_[0], buffer_.size());                 
    // 重置读写位置，使缓冲区可以重新写入新的数据
    readPos_ = 0;
    writePos_ = 0;
}

// 从缓冲区中取出所有数据到一个字符串中
std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();                                  // 清空缓冲区中的数据
    return str;                                     // 返回取出的字符串
}

// 返回可写数据的起始位置（只读）
const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

// 返回可写数据的起始位置
char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

// 更新已写入数据的位置
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

// 向缓冲区追加字符串
void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

// 向缓冲区追加二进制数据
void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

// 追加指定长度的字符串到缓冲区
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

// 追加另一个缓冲区的数据到当前缓冲区
void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

// 如果可写空间不足，扩充缓冲区
void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}

// 从文件描述符中读取数据到缓冲区
ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];                               // 临时缓冲区
    struct iovec iov[2];                            // 用于readv函数的两个iovec结构体，iov[0]用于写入数据到缓冲区，iov[1]用于保存多余的数据
    const size_t writable = WritableBytes();        // 设置iov[0]，指向当前可写位置，长度为可写字节数
    iov[0].iov_base = BeginPtr_() + writePos_;      // 分散读， 保证数据全部读完
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;                         // 设置iov[1]，指向临时缓冲区，长度为临时缓冲区的大小
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);          // 从文件描述符fd中读取数据到缓冲区
    if(len < 0) {
        *saveErrno = errno;                         // 处理读取结果
    }
    else if(static_cast<size_t>(len) <= writable) {
        writePos_ += len;                           // 更新写入位置
    }
    else {
        writePos_ = buffer_.size();
        Append(buff, len - writable);               // 写入多余的数据到缓冲区
    }
    return len;
}

// 将缓冲区的数据写入文件描述符
ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();              // 获取可读数据大小
    ssize_t len = write(fd, Peek(), readSize);      // 将缓冲区中的数据写入文件描述符fd
    if(len < 0) {
        *saveErrno = errno;                         // 处理写入结果
        return len;
    } 
    readPos_ += len;                                // 更新读取位置
    return len;
}

// 返回缓冲区的起始地址
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

// 返回只读缓冲区的起始地址
const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

// 如果剩余的可写和可读空间都不足以容纳len大小的数据，则进行扩容
void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {// 已经读取的数据 + 还需写入的数据 < len
        buffer_.resize(writePos_ + len + 1);       
    } 
    else {                                          // 将未读数据向前移动，以便腾出空间
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}