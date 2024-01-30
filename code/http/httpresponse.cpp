#include "httpresponse.h"
using namespace std;

// 文件后缀名到MIME类型的映射
const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

// HTTP状态码到状态消息的映射
const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

// 特定状态码到错误页面文件路径的映射
const unordered_map<int, string> HttpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

// 构造函数
HttpResponse::HttpResponse() {
    code_ = -1;                             // 初始化HTTP状态码为-1
    path_ = srcDir_ = "";                   // 初始化路径和源目录为空字符串
    isKeepAlive_ = false;                   // 初始化为非持久连接
    mmFile_ = nullptr;                      // 初始化内存映射文件指针为null
    mmFileStat_ = { 0 };                    // 初始化文件状态结构
};

// 析构函数
HttpResponse::~HttpResponse() {
    UnmapFile();                            // 调用UnmapFile解除文件映射
}

// 初始化HttpResponse对象
void HttpResponse::Init(const string& srcDir, string& path, bool isKeepAlive, int code){
    assert(srcDir != "");                   // 断言源目录非空
    if(mmFile_) { UnmapFile(); }            // 如果已有文件映射，则解除映射
    code_ = code;                           // 设置HTTP状态码
    isKeepAlive_ = isKeepAlive;             // 设置连接是否保持活跃
    path_ = path;                           // 设置请求路径
    srcDir_ = srcDir;                       // 设置源文件目录
    mmFile_ = nullptr;                      // 重置内存映射文件指针
    mmFileStat_ = { 0 };                    // 重置文件状态结构
}

// 构建HTTP响应
void HttpResponse::MakeResponse(Buffer& buff) {
    // 判断请求的资源文件是否存在且可访问
    if(stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404;                        // 如果文件不存在或是目录，则设置状态码为404
    }
    else if(!(mmFileStat_.st_mode & S_IROTH)) {
        code_ = 403;                        // 如果文件没有读权限，则设置状态码为403
    }
    else if(code_ == -1) { 
        code_ = 200;                        // 如果之前没有设置状态码，则默认为200
    }
    ErrorHtml_();                           // 根据状态码设置错误页面
    AddStateLine_(buff);                    // 添加状态行到响应缓冲区
    AddHeader_(buff);                       // 添加响应头部
    AddContent_(buff);                      // 添加响应内容
}

// 返回内存映射的文件数据
char* HttpResponse::File() {
    return mmFile_;
}

// 获取文件长度
size_t HttpResponse::FileLen() const {
    return mmFileStat_.st_size;
}

// 设置错误HTML页面
void HttpResponse::ErrorHtml_() {
    if(CODE_PATH.count(code_) == 1) {
        // 如果有对应状态码的错误页面，则设置路径
        path_ = CODE_PATH.find(code_)->second;
        // 获取错误页面文件的状态
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}

// 添加状态行
void HttpResponse::AddStateLine_(Buffer& buff) {
    string status;
    if(CODE_STATUS.count(code_) == 1) {
        // 获取状态码对应的状态消息
        status = CODE_STATUS.find(code_)->second;
    }
    else {
        // 如果状态码未知，则设置为400 Bad Request
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    // 将状态行添加到缓冲区
    buff.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

// 添加响应头
void HttpResponse::AddHeader_(Buffer& buff) {
    buff.Append("Connection: ");            // 添加Connection头部
    if(isKeepAlive_) {
        buff.Append("keep-alive\r\n");      // 如果是保持连接，添加keep-alive
        // 设置keep-alive的参数
        buff.Append("keep-alive: max=6, timeout=120\r\n"); 
    } else {
        buff.Append("close\r\n");           // 如果不是保持连接，添加close
    }
    // 添加Content-type头部，根据文件类型设置
    buff.Append("Content-type: " + GetFileType_() + "\r\n"); 
}

// 添加响应内容
void HttpResponse::AddContent_(Buffer& buff) {
    // 打开请求的文件
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY); 
    if(srcFd < 0) { 
        ErrorContent(buff, "File NotFound!"); 
        return; 
    }
    // 将文件映射到内存以提高访问速度
    LOG_DEBUG("file path %s", (srcDir_ + path_).data());
    int* mmRet = (int*)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mmRet == -1) {
        ErrorContent(buff, "File NotFound!");
        return; 
    }
    mmFile_ = (char*)mmRet;                 // 保存映射的地址
    close(srcFd); // 关闭文件描述符
    // 添加Content-length头部
    buff.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n"); 
}

// 解除文件映射
void HttpResponse::UnmapFile() {
    if(mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size); 
        mmFile_ = nullptr;
    }
}

// 获取响应的文件类型
string HttpResponse::GetFileType_() {
    // 查找路径中最后一个点的位置
    string::size_type idx = path_.find_last_of('.');
    if(idx == string::npos) {
        return "text/plain";                // 如果没有找到文件扩展名，返回"text/plain"
    }
    string suffix = path_.substr(idx);      // 提取文件后缀
    // 查找文件后缀对应的MIME类型
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";                    // 如果后缀未知，返回"text/plain"
}

// 生成错误内容
void HttpResponse::ErrorContent(Buffer& buff, string message) {
    string body;                            // 错误页面的HTML内容
    string status;
    // 构建HTML页面
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    // 获取并显示状态消息
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    // 将错误页面的内容长度添加到响应头部
    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    // 将错误页面的HTML内容添加到响应缓冲区
    buff.Append(body); 
}