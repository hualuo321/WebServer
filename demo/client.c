#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
    // 1. 创建客户端 socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 连接服务端
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.110.129", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(fd, (struct sockaddr*)& serveraddr, sizeof(serveraddr));
    if (ret == -1) {
        perror("connect");
        exit(-1);
    }
    // 3. 通信
    char recvBuf[1024] = {0};
    while(1) {
        char* data = "hello, i am client";
        // 给服务端发送数据
        write(fd, data, strlen);
        // 获取服务端的数据
        int len = read(fd, recvBuf, sizeof(recvBuf));
        if (len == -1) {
            perror("read");
            exit(-1);
        } else if (len > 0) {
            printf("recv server data : %s\n", recvBuf);
        } else {
            printf("server closed ...");
            break;
        }
        sleep(1);
    }

    // 关闭文件描述符
    close(fd);
    return 0;
}