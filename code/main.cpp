/*
 * @Author       : mark
 * @Date         : 2020-06-18
 * @copyleft Apache 2.0
 */ 
#include <unistd.h>
#include "server/webserver.h"

int main() {
    //daemon(1, 0); 

    WebServer server(
        1316, 3, 60000, false, 
        3306, "root", "root", "webserver", 
        12, 6, true, 1, 1024);             
    server.Start();
} 
  