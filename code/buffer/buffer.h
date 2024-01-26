#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>                              // ����ʹ�� perror ����
#include <iostream>
#include <unistd.h>                             // ����ʹ�� write ����
#include <sys/uio.h>                            // ����ʹ�� readv ����
#include <vector>                      
#include <atomic>
#include <assert.h>

// �������� ����
class Buffer {
public:
    // ���캯������ʼ����������С��Ĭ��Ϊ1024�ֽ�
    Buffer(int initBuffSize = 1024);
    // Ĭ����������
    ~Buffer() = default;
    // ���ؿ�д���ֽ���
    size_t WritableBytes() const;
    // ���ؿɶ����ֽ���
    size_t ReadableBytes() const ;
    // ����Ԥ�������ֽ���
    size_t PrependableBytes() const;
    // ��ȡ������������ʼλ�õ�ָ��
    const char* Peek() const;
    // ȷ�����������㹻�Ŀ�д�ռ�
    void EnsureWriteable(size_t len);
    // ���»�������д����ֽ���
    void HasWritten(size_t len);
    // ��ȡָ�����ȵ�����
    void Retrieve(size_t len);
    // ��ȡֱ��ָ��λ�õ�����
    void RetrieveUntil(const char* end);
    // ��ȡȫ������
    void RetrieveAll() ;
    // ��ȡȫ�����ݲ�����Ϊ�ַ���
    std::string RetrieveAllToStr();
    // ��ȡ��д������ʼλ�õ�ָ�루��const�汾��
    char* BeginWrite();
    // ��ȡ��д������ʼλ�õ�ָ�루const�汾��
    const char* BeginWriteConst() const;
    // ׷���ַ�����������
    void Append(const std::string& str);
    // ׷��ָ�����ȵ��ַ�����������
    void Append(const char* str, size_t len);
    // ׷��ָ�����ȵĶ��������ݵ�������
    void Append(const void* data, size_t len);
    // ׷����һ�������������ݵ���ǰ������
    void Append(const Buffer& buff);
    // ���ļ��������ж�ȡ���ݵ�������
    ssize_t ReadFd(int fd, int* Errno);
    // ��������������д���ļ�������
    ssize_t WriteFd(int fd, int* Errno);

private:
    // ��ȡ������������ʼλ�õ�ָ�루��const�汾��
    char* BeginPtr_();
    // ��ȡ������������ʼλ�õ�ָ�루const�汾��
    const char* BeginPtr_() const;
    // ȷ�����������㹻�Ŀռ�
    void MakeSpace_(size_t len);
    // ���������ݴ洢������
    std::vector<char> buffer_;
    // ��ȡλ�õ�ԭ�ӱ���
    std::atomic<std::size_t> readPos_;
    // д��λ�õ�ԭ�ӱ���
    std::atomic<std::size_t> writePos_;
};

#endif //BUFFER_H