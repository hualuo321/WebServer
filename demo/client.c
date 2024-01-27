#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
    // 1. �����ͻ��� socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. ���ӷ����
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.110.129", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(fd, (struct sockaddr*)& serveraddr, sizeof(serveraddr));
    if (ret == -1) {
        perror("connect");
        exit(-1);
    }
    // 3. ͨ��
    char recvBuf[1024] = {0};
    while(1) {
        char* data = "hello, i am client";
        // ������˷�������
        write(fd, data, strlen);
        // ��ȡ����˵�����
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

    // �ر��ļ�������
    close(fd);
    return 0;
}