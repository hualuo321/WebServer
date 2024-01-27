#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
    // 1. ��������� socket ���ڼ���
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. ��
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;             // Э����
    saddr.sin_addr.s_addr = INADDR_ANY;     // IP
    saddr.sin_port = htons(9999);           // �˿ں�
    int ret = bind(lfd, (struct sockaddr*)& saddr, sizeof(saddr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 3. ����
    ret = listen(lfd, 8);
    if (ret == -1) {
        perror("listen");
        exit(-1);
    }
    // 4. ���տͻ�������
    struct sockaddr_in clientaddr;
    int len = sizeof(clientaddr);
    int cfd = accept(lfd, (struct sockaddr*)& clientaddr, &len);
    if (cfd == -1) {
        perror("accept");
        exit(-1);
    }
    // ����ͻ�����Ϣ
    char clientIP[16];
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned short clinetPort = ntohs(clientaddr.sin_port);
    printf("client ip is %d", clientIP, clinetPort);
    // 5. ͨ��
    char recvBuf[1024] = {0};
    while(1) {
        // ��ȡ�ͻ��˵�����
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
        char* data = "hello, i am server";
        // ���ͻ��˷�������
        write(cdf, data, strlen(data));
    }

    // �ر��ļ�������
    close(cfd);
    close(lfd);
    return 0;
}