#include "buffer.h"

// ���캯������ʼ���������Ͷ�дλ��
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

// ����ʣ��ɶ��ֽ���
size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}

// ���ؿ�д�ֽ���
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

// ��ʾ�Ѿ���ȡ�����ݴ�С
size_t Buffer::PrependableBytes() const {
    return readPos_;
}

// ���ػ������пɶ����ݵ�ָ��
const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

// �ӻ�������ȡ��ָ�����ȵ�����, ������ָ����ǰ�ƶ�
void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}

// �ӻ�������ȡ�����ݣ�ֱ��ָ���Ľ���λ��
void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

// ��ջ���������������������ȫ������Ϊ��ʼ״̬���Ա�����ʹ��
void Buffer::RetrieveAll() {
    // ʹ�� bzero ������������������ȫ����Ϊ�㣬�൱����ջ�����
    bzero(&buffer_[0], buffer_.size());                 
    // ���ö�дλ�ã�ʹ��������������д���µ�����
    readPos_ = 0;
    writePos_ = 0;
}

// �ӻ�������ȡ���������ݵ�һ���ַ�����
std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();                                  // ��ջ������е�����
    return str;                                     // ����ȡ�����ַ���
}

// ���ؿ�д���ݵ���ʼλ�ã�ֻ����
const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

// ���ؿ�д���ݵ���ʼλ��
char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

// ������д�����ݵ�λ��
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

// �򻺳���׷���ַ���
void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

// �򻺳���׷�Ӷ���������
void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

// ׷��ָ�����ȵ��ַ�����������
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

// ׷����һ�������������ݵ���ǰ������
void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

// �����д�ռ䲻�㣬���仺����
void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}

// ���ļ��������ж�ȡ���ݵ�������
ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];                               // ��ʱ������
    struct iovec iov[2];                            // ����readv����������iovec�ṹ�壬iov[0]����д�����ݵ���������iov[1]���ڱ�����������
    const size_t writable = WritableBytes();        // ����iov[0]��ָ��ǰ��дλ�ã�����Ϊ��д�ֽ���
    iov[0].iov_base = BeginPtr_() + writePos_;      // ��ɢ���� ��֤����ȫ������
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;                         // ����iov[1]��ָ����ʱ������������Ϊ��ʱ�������Ĵ�С
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);          // ���ļ�������fd�ж�ȡ���ݵ�������
    if(len < 0) {
        *saveErrno = errno;                         // �����ȡ���
    }
    else if(static_cast<size_t>(len) <= writable) {
        writePos_ += len;                           // ����д��λ��
    }
    else {
        writePos_ = buffer_.size();
        Append(buff, len - writable);               // д���������ݵ�������
    }
    return len;
}

// ��������������д���ļ�������
ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();              // ��ȡ�ɶ����ݴ�С
    ssize_t len = write(fd, Peek(), readSize);      // ���������е�����д���ļ�������fd
    if(len < 0) {
        *saveErrno = errno;                         // ����д����
        return len;
    } 
    readPos_ += len;                                // ���¶�ȡλ��
    return len;
}

// ���ػ���������ʼ��ַ
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

// ����ֻ������������ʼ��ַ
const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

// ���ʣ��Ŀ�д�Ϳɶ��ռ䶼����������len��С�����ݣ����������
void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {// �Ѿ���ȡ������ + ����д������� < len
        buffer_.resize(writePos_ + len + 1);       
    } 
    else {                                          // ��δ��������ǰ�ƶ����Ա��ڳ��ռ�
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}