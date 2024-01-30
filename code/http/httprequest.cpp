#include "httprequest.h"
using namespace std;

// 默认的HTML文件集合
const unordered_set<string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

// 默认HTML文件对应的标签
const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

// 初始化HTTP请求
void HttpRequest::Init() {
    method_ = path_ = version_ = body_ = "";        // 初始化方法、路径、版本和主体为空字符串
    state_ = REQUEST_LINE;                          // 设置初始解析状态为请求行
    header_.clear();                                // 清空头部字段
    post_.clear();                                  // 清空POST数据
}

// 判断是否保持连接
bool HttpRequest::IsKeepAlive() const {
    // 检查Connection头部字段是否为keep-alive，并且版本为1.1
    if(header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

// 解析HTTP请求
bool HttpRequest::parse(Buffer& buff) {
    const char CRLF[] = "\r\n";                     // 行结束标识
    if(buff.ReadableBytes() <= 0) {
        return false;
    }
    while(buff.ReadableBytes() && state_ != FINISH) {
        const char* lineEnd = search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.Peek(), lineEnd);     // 获取一行数据
        switch(state_)
        {
        case REQUEST_LINE:
            if(!ParseRequestLine_(line)) {          // 解析请求行失败
                return false;
            }
            ParsePath_();                           // 解析路径
            break;    
        case HEADERS:
            ParseHeader_(line);                     // 解析头部字段
            if(buff.ReadableBytes() <= 2) {
                state_ = FINISH;
            }
            break;
        case BODY:
            ParseBody_(line);                       // 解析主体
            break;
        default:
            break;
        }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);            // 移动缓冲区读取位置
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

// 解析路径
void HttpRequest::ParsePath_() {
    if(path_ == "/") {
        path_ = "/index.html";                      // 默认路径
    }
    else {
        for(auto &item: DEFAULT_HTML) {
            if(item == path_) {
                path_ += ".html";                   // 添加默认HTML文件后缀
                break;
            }
        }
    }
}

// 解析HTTP请求行
bool HttpRequest::ParseRequestLine_(const string& line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$"); // 正则表达式，匹配请求行
    smatch subMatch;                                // 用于存储匹配结果
    if(regex_match(line, subMatch, patten)) {   
        method_ = subMatch[1];                      // 获取请求方法
        path_ = subMatch[2];                        // 获取请求路径
        version_ = subMatch[3];                     // 获取HTTP版本
        state_ = HEADERS;                           // 更新解析状态为HEADERS
        return true;
    }
    LOG_ERROR("RequestLine Error");                 // 请求行格式错误
    return false;
}

// 解析HTTP头部
void HttpRequest::ParseHeader_(const string& line) {
    regex patten("^([^:]*): ?(.*)$");               // 正则表达式，匹配头部字段
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        header_[subMatch[1]] = subMatch[2];         // 存储头部字段
    }
    else {
        state_ = BODY;                              // 头部解析完成，更新解析状态为BODY
    }
}

// 解析HTTP主体
void HttpRequest::ParseBody_(const string& line) {
    body_ = line;                                   // 获取主体内容
    ParsePost_();                                   // 解析POST数据
    state_ = FINISH;                                // 更新解析状态为FINISH
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

// 将十六进制字符转换为十进制数
int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

// 解析POST请求
void HttpRequest::ParsePost_() {
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded_();                     // 解析application/x-www-form-urlencoded类型的数据
        // 检查路径是否在默认的HTML标签中
        if(DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            // 根据标签处理登录或注册请求
            bool isLogin = (tag == 1);
            if(UserVerify(post_["username"], post_["password"], isLogin)) {
                path_ = "/welcome.html";            // 验证成功
            } 
            else {
                path_ = "/error.html";              // 验证失败
            }
        }
    }
}

// 解析application/x-www-form-urlencoded类型的POST请求体
void HttpRequest::ParseFromUrlencoded_() {
    if(body_.size() == 0) { return; }               // 如果请求体为空，则直接返回

    string key, value;
    int num = 0;
    int n = body_.size();                           // 请求体长度
    int i = 0, j = 0;                               // i用于遍历，j用于记录键或值的起始位置

    for(; i < n; i++) {
        char ch = body_[i];                         // 当前字符
        switch (ch) {
        case '=':
            key = body_.substr(j, i - j);           // 提取键
            j = i + 1;                              // 更新下一个值的起始位置
            break;
        case '+':
            body_[i] = ' ';                         // 将+替换为空格
            break;
        case '%':
            // 处理百分号编码的字符
            num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = body_.substr(j, i - j);         // 提取值
            j = i + 1;                              // 更新下一个键的起始位置
            post_[key] = value;                     // 将键值对存入post_字典
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    // 处理最后一个键值对
    if(post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

// 用户验证
bool HttpRequest::UserVerify(const string &name, const string &pwd, bool isLogin) {
    if(name == "" || pwd == "") { return false; }   // 如果用户名或密码为空，返回false
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnRAII(&sql,  SqlConnPool::Instance());    // 获取数据库连接
    assert(sql);                                    // 确保数据库连接有效

    bool flag = false;
    // unsigned int j = 0;
    char order[256] = { 0 };
    // MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;
    
    if(!isLogin) { flag = true; }                   // 如果不是登录操作，设置flag为true
    // 查询用户名和密码
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);

    if(mysql_query(sql, order)) { 
        mysql_free_result(res);
        return false; 
    }
    res = mysql_store_result(sql);
    // j = mysql_num_fields(res);
    // fields = mysql_fetch_fields(res);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        string password(row[1]);
        // 检查密码是否匹配
        if(isLogin) {
            if(pwd == password) { flag = true; }
            else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        } else { 
            flag = false; 
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    // 如果是注册操作且用户名未被使用
    if(!isLogin && flag == true) {
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "Insert error!");
            flag = false; 
        }
        flag = true;
    }
    SqlConnPool::Instance()->FreeConn(sql);         // 释放数据库连接
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}

// 获取请求路径
std::string HttpRequest::path() const {
    return path_;
}

std::string& HttpRequest::path() {
    return path_;
}

// 获取请求方法
std::string HttpRequest::method() const {
    return method_;
}

// 获取HTTP版本
std::string HttpRequest::version() const {
    return version_;
}

// 获取POST请求的参数
std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}
