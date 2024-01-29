#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>                                          // 文件控制，例如 open
#include <unistd.h>                                         // UNIX标准符号常量和类型，例如 close
#include <sys/stat.h>                                       // 文件状态信息，例如 stat
#include <sys/mman.h>                                       // 内存管理声明，例如 mmap, munmap

#include "../buffer/buffer.h"
#include "../log/log.h"

// HttpResponse 类，用于处理HTTP响应
class HttpResponse {
public:
    HttpResponse();                                         // 构造函数
    ~HttpResponse();                                        // 析构函数
    // 初始化响应
    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff);                        // 构造响应
    void UnmapFile();                                       // 解除文件映射
    char* File();                                           // 获取文件地址
    size_t FileLen() const;                                 // 获取文件长度
    void ErrorContent(Buffer& buff, std::string message);   // 生成错误内容
    int Code() const { return code_; }                      // 获取响应码

private:
    void AddStateLine_(Buffer &buff);                       // 添加响应行
    void AddHeader_(Buffer &buff);                          // 添加响应头
    void AddContent_(Buffer &buff);                         // 添加响应体

    void ErrorHtml_();                                      // 生成错误页面
    std::string GetFileType_();                             // 获取文件类型

    int code_;                                              // HTTP响应码
    bool isKeepAlive_;                                      // 是否保持连接

    std::string path_;                                      // 请求路径
    std::string srcDir_;                                    // 源文件目录
    
    char* mmFile_;                                          // 内存映射的文件数据
    struct stat mmFileStat_;                                // 文件状态信息
    // 文件后缀到MIME类型的映射
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    // 状态码到状态信息的映射
    static const std::unordered_map<int, std::string> CODE_STATUS;
    // 状态码到错误页面路径的映射
    static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif //HTTP_RESPONSE_H