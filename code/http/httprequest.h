#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

// HttpRequest 类，用于解析HTTP请求
class HttpRequest {
public:
    // 解析状态枚举
    enum PARSE_STATE {
        REQUEST_LINE,                               // 请求行
        HEADERS,                                    // 请求头
        BODY,                                       // 请求体
        FINISH,                                     // 完成
    };
    // HTTP响应代码枚举
    enum HTTP_CODE {
        NO_REQUEST = 0,                             // 无请求
        GET_REQUEST,                                // GET请求
        BAD_REQUEST,                                // 错误请求
        NO_RESOURSE,                                // 无资源
        FORBIDDENT_REQUEST,                         // 禁止请求
        FILE_REQUEST,                               // 文件请求
        INTERNAL_ERROR,                             // 内部错误
        CLOSED_CONNECTION,                          // 关闭连接
    };
    
    HttpRequest() { Init(); }                       // 构造函数
    ~HttpRequest() = default;                       // 析构函数

    void Init();                                    // 初始化HttpRequest对象
    bool parse(Buffer& buff);                       // 解析HTTP请求

    std::string path() const;                       // 获取请求路径
    std::string& path();                
    std::string method() const;                     // 获取请求方法
    std::string version() const;                    // 获取HTTP版本
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;     // 获取POST请求的数据

    bool IsKeepAlive() const;                       // 判断连接是否保持活跃

private:
    bool ParseRequestLine_(const std::string& line);// 解析请求行
    void ParseHeader_(const std::string& line);     // 解析请求头
    void ParseBody_(const std::string& line);       // 解析请求体
    void ParsePath_();                              // 解析路径
    void ParsePost_();                              // 解析POST请求
    void ParseFromUrlencoded_();                    // 解析application/x-www-form-urlencoded格式的数据
    // 用户验证（用于登录或注册等功能）
    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE state_;                             // 当前解析状态
    std::string method_, path_, version_, body_;    // 请求方法、路径、版本和主体
    // 头部信息
    std::unordered_map<std::string, std::string> header_;
    // POST数据
    std::unordered_map<std::string, std::string> post_;
    // 默认的HTML文件集合
    static const std::unordered_set<std::string> DEFAULT_HTML;
    // 默认的HTML标签
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    // 将十六进制字符转换为十进制
    static int ConverHex(char ch);
};

#endif //HTTP_REQUEST_H