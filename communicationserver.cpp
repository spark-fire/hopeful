#include "communicationserver.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <unistd.h> //close()

#include "protobuf/person.pb.h"

#define SERVER_PORT      12306
#define MAX_CLIENT_COUNT 5
#define MAXLIN           4096

CommunicationServer::CommunicationServer()
{
}

bool CommunicationServer::serviceLoopStart()
{
    bool ret = false;
    if (initSocket()) {
        //创建事件循环对象(libev机制)
        m_eventloop = ev_loop_new(EVBACKEND_EPOLL);
        if (!m_eventloop) {
            std::cout << "create eventloop failed" << std::endl;
        }

        ev_io_init(&m_connectIoWatcher, acceptNewUserCallback, m_listenFd,
                   EV_READ);
        ev_io_start(m_eventloop, &m_connectIoWatcher);
        ev_loop(m_eventloop, 0); // loop forever
        std::cout << "exit eventloop " << std::endl;

        ev_loop_destroy(m_eventloop);
        ret = true;
    }

    return ret;
}

bool CommunicationServer::initSocket()
{
    bool ret = false;
    int sockServer = -1;
    int reuseaddr = 1;
    struct sockaddr_in addrClient;
    memset(&addrClient, 0, sizeof(addrClient));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    // 01 create socket
    sockServer = socket(AF_INET, SOCK_STREAM, 0);
    if (sockServer > 0) {
        ret = true;
        m_listenFd = sockServer;

        // set port reuse
        setsockopt(sockServer, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                   sizeof(reuseaddr));

        // 02 init server, fullfill sockaddr_in
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_PORT);
        addr.sin_addr.s_addr = htonl(INADDR_ANY); // receive data from any ip

        // 03 bind socket
        if (bind(sockServer, (struct sockaddr *)&addr,
                 sizeof(struct sockaddr)) < 0) {
            std::cout << "bind faild! error is " << strerror(errno)
                      << std::endl;
            m_listenFd = -1;
            ret = false;
        }

        // 04 set the socket to listen mode, ready to receive client requests
        if (listen(sockServer, MAX_CLIENT_COUNT) < 0) {
            std::cout << "listen faild! error is " << strerror(errno)
                      << std::endl;
            m_listenFd = -1;
            ret = false;
        }

        m_listenFd = sockServer;

    } else {
        std::cout << "create socket server faild! error is " << strerror(errno)
                  << std::endl;
        m_listenFd = -1;
        ret = false;
    }

    return ret;
}

void CommunicationServer::acceptNewUserCallback(struct ev_loop *loop, ev_io *w,
                                                int events)
{
    (void)loop;
    (void)events;

    int newfd = -1;

    struct sockaddr_in clientAddress;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    ev_io *acceptWatcher = (ev_io *)malloc(sizeof(ev_io));

    if (EV_ERROR & events) {
        std::cout << "accept got invalid event..." << std::endl;
        return;
    }
    // accept连接
    newfd = accept(w->fd, (struct sockaddr *)&clientAddress, &addrlen);
    if (-1 == newfd) {
        std::cout << "socket accept faild! error is " << strerror(errno)
                  << std::endl;
        return;
    }
    //设置非阻塞
    if (-1 == fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL) | O_NONBLOCK)) {
        close(newfd);
        return;
    }
    printf("Successfully connected with client: %s:%u\r\n",
           inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);

    //加入事件循环
    ev_io_init(acceptWatcher, recvUserMsgCallback, newfd, EV_READ);
    ev_io_start(loop, acceptWatcher);

    return;
}

void CommunicationServer::recvUserMsgCallback(struct ev_loop *loop, ev_io *w,
                                              int events)
{
    char buffer[1024] = { 0 };
    if (EV_ERROR & events) {
        printf("read got invalid event...\r\n");
        return;
    }
    int res = 0;
    int32_t bytes = read(w->fd, buffer, sizeof(buffer));
    if (-1 == bytes) {
        // tcp Error
        if (EINTR != errno && EAGAIN != errno) {
            res = 1;
        }
    } else if (0 == bytes) {
        // tcp Close
        res = 2;
    }

    if (0 != res) {
        //关闭事件循环并释放watcher
        printf("TCP CLOSE\r\n");
        ev_io_stop(loop, w);
        free(w);
    } else {
        printf("READ:\r\n    %s\r\n", buffer);

        //反序列化
        std::string recv(buffer);
        Test::Person p1;
        p1.ParseFromString(recv);
        std::cout << p1.id() << std::endl;
        std::cout << p1.name() << std::endl;
        std::cout << p1.email() << std::endl;
    }
}
