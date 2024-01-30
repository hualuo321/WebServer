#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
    // 1. 创建服务端 socket 用于监听
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 绑定 IP 端口到 lfd
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;             // 协议族
    saddr.sin_addr.s_addr = INADDR_ANY;     // IP
    saddr.sin_port = htons(9999);           // 端口号
    int ret = bind(lfd, (struct sockaddr*)& saddr, sizeof(saddr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 3. 监听
    ret = listen(lfd, 8);
    if (ret == -1) {
        perror("listen");
        exit(-1);
    }
    // 4. 接收客户端连接
    struct sockaddr_in clientaddr;
    int len = sizeof(clientaddr);
    // 客户端 lfd 进行阻塞等待, 把等待到的连接用户域名端口信息放入到 clientaddr, 并返回其 cfd
    int cfd = accept(lfd, (struct sockaddr*)& clientaddr, &len);
    if (cfd == -1) {
        perror("accept");
        exit(-1);
    }
    // 输出客户端信息
    char clientIP[16];
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned short clinetPort = ntohs(clientaddr.sin_port);
    printf("client ip is %s, port is %d\n", clientIP, clinetPort);
    // 5. 通信
    char recvBuf[1024] = {0};
    while(1) {
        // 获取客户端的数据
        int num = read(cfd, recvBuf, sizeof(recvBuf));
        if (num == -1) {
            perror("read");
            exit(-1);
        } else if (num > 0) {
            printf("recv client data : %s\n", recvBuf);
        } else {
            printf("client closed ...");
            break;
        }
        // 给客户端发送数据
        char* data = "hello, i am server";
        write(cfd, data, strlen(data));
    }

    // 关闭文件描述符
    close(cfd);
    close(lfd);
    return 0;
}