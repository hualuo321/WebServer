#include <unistd.h>
#include "server/webserver.h"

int main() {
    // 守护进程， 后台运行
    // daemon(1, 0); 

    // 服务器初始化 (port, ET, timeout, 优雅退出, sqlPort, user, passwd, sqlName, connNum, threadNum, 日志, 日志等级, 日志队列大小)
    WebServer server(1316, 3, 60000, false, 3306, "root", "root", "webserver", 12, 6, true, 1, 1024);             
    server.Start();
} 
  