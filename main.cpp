

#include <sys/socket.h>

#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#include <iostream>

#include "communicationserver.h"

#define SERVER_PORT      12306
#define MAX_CLIENT_COUNT 5
#define MAXLIN           4096

int main()
{
    //    int sockServer = -1;
    //    int connfd = -1;
    //    struct sockaddr_in addrClient;
    //    memset(&addrClient, 0, sizeof(addrClient));

    //    // 01 create socket
    //    sockServer = socket(AF_INET, SOCK_STREAM, 0);
    //    if (sockServer < 0) {
    //        std::cout << "create socket server faild! error is " <<
    //        strerror(errno)
    //                  << std::endl;
    //        return -1;
    //    }

    //    // 02 init server, fullfill sockaddr_in
    //    struct sockaddr_in addr;
    //    memset(&addr, 0, sizeof(addr));
    //    addr.sin_family = AF_INET;
    //    addr.sin_port = htons(SERVER_PORT);
    //    addr.sin_addr.s_addr = htonl(INADDR_ANY); // receive data from any ip

    //    // set port reuse
    //    int reuseaddr = 1;
    //    setsockopt(sockServer, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
    //               sizeof(reuseaddr));

    //    // 03 bind socket
    //    if (bind(sockServer, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    //        std::cout << "bind faild! error is " << strerror(errno) <<
    //        std::endl; return -1;
    //    }

    //    // 04 set the socket to listen mode, ready to receive client requests
    //    if (listen(sockServer, MAX_CLIENT_COUNT) < 0) {
    //        std::cout << "listen faild! error is " << strerror(errno) <<
    //        std::endl; return -1;
    //    }

    //    socklen_t sin_size = sizeof(struct sockaddr_in);

    //    const int nTempLen = 4096;
    //    char *recvbuf = (char *)malloc((size_t)nTempLen);

    //    while (1) {
    //        std::cout << "wait client connecting ...... " << std::endl;
    //        connfd =
    //            accept(sockServer, (struct sockaddr *)&(addrClient),
    //            &sin_size);
    //        printf(" client connect success !!! ip is: %s port is: %u \n",
    //               (inet_ntoa)(addrClient.sin_addr), addrClient.sin_port);

    //        while (connfd) {
    //            int nRecv = recv(connfd, recvbuf, nTempLen, 0);
    //            printf("recv size is %d \n", nRecv);

    //            if (nRecv <= 0 && errno != EINTR) {
    //                printf(" client disconnect !\n");
    //                break;
    //            }

    //            if (nRecv > 1) {
    //                printf("%s \n", recvbuf);

    //                if (send(connfd, recvbuf, nTempLen, 0) < 0) {
    //                    printf("send msg error :%s(errno:%d)\n",
    //                    strerror(errno),
    //                           errno);
    //                }
    //            }
    //            memset(recvbuf, 0, strlen(recvbuf));
    //        }
    //    }

    //    puts("!!!Hello World!!!");
    //    return EXIT_SUCCESS;

    CommunicationServer s;
    s.serviceLoopStart();
}
