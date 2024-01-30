#include <unistd.h>
#include "server/webserver.h"

int main() {
    // 当控制终端被关闭时，相应的进程都会自动关闭。若不想让进程因终端的关闭而受影响，需要把进程变为守护进程。
    // daemon(1, 0); 

    // 服务器初始化 (port, ET, timeout, optLinger, sqlPort, user, passwd, sqlName, sqlNum, threadNum, Log, LogLevel, LogQueSize)
    WebServer server(1316, 3, 60000, false, 3306, "root", "Xch990705", "webserver", 12, 6, true, 1, 1024);     

    // 服务器启动        
    server.Start();
}