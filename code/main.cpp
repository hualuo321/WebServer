#include <unistd.h>
#include "server/webserver.h"

int main() {
    // 服务器初始化 (port, ET, timeout, optLinger, sqlPort, user, passwd, sqlName, sqlNum, threadNum, Log, LogLevel, LogQueSize)
    WebServer server(1316, 3, 60000, false, 3306, "root", "Xch990705", "webserver", 12, 6, true, 1, 1024);     
    // 服务器启动        
    server.Start();
}