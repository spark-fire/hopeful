#include "communicationserver.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <unistd.h> //close()
#include "global.h"

#include "protobuf/person.pb.h"

#define SERVER_PORT      12306
#define MAX_CLIENT_COUNT 5
#define MAXLIN           4096

CommunicationServer *CommunicationServer::s_singletonHandle = NULL;

CommunicationServer::CommunicationServer()
{
    init();
    s_singletonHandle = this;
}

bool CommunicationServer::serviceLoopStart()
{
    bool ret = false;
    if (initSocket()) {
        //创建事件循环对象(libev机制)
        m_eventloop = ev_loop_new(EVBACKEND_EPOLL);
        if (!m_eventloop) {
            W_ERROR("create eventloop failed");
        }

        ev_io_init(&m_connectIoWatcher, acceptNewUserCallback, m_listenFd,
                   EV_READ);
        ev_io_start(m_eventloop, &m_connectIoWatcher);
        ev_loop(m_eventloop, 0); // loop forever
        W_INFO("exit eventloop ");

        ev_loop_destroy(m_eventloop);
        ret = true;
    }

    return ret;
}

void CommunicationServer::init()
{
    m_clientObjectMap.clear();

    m_listenFd = INVALID_SOCKET;

    m_eventloop = NULL;

    pthread_mutex_init(&m_operateClientObjectMapMutex, NULL);
}

void CommunicationServer::initClientObject(
    CommunicationServer::ClientObject &client, ev_io *pWatcher)
{
    client.pEvWatcher = pWatcher;
    client.pRecvBuffer = new char[RECV_BUFFER_SIZE];
    client.pSendBuffer = new char[SEND_BUFFER_SIZE];

    client.connectState = ConnectNotLogin;
    time(&client.connectTimestamp);
    client.isReserve = false;

    pthread_mutex_init(&client.sendMutex, NULL);
}

void CommunicationServer::uninitClientObject(
    CommunicationServer::ClientObject &client)
{
    if (client.pEvWatcher) {
        free(client.pEvWatcher);
        client.pEvWatcher = NULL;
    }

    if (client.pRecvBuffer) {
        delete client.pRecvBuffer;
        client.pRecvBuffer = NULL;
    }

    if (client.pSendBuffer) {
        delete client.pSendBuffer;
        client.pSendBuffer = NULL;
    }

    client.connectState = Disconnect;
    client.isReserve = false;
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
            W_ERROR("bind faild! error is %s ", strerror(errno));
            m_listenFd = -1;
            ret = false;
        }

        // 04 set the socket to listen mode, ready to receive client requests
        if (listen(sockServer, MAX_CLIENT_COUNT) < 0) {
            W_ERROR("listen faild! error is %s", strerror(errno));
            m_listenFd = -1;
            ret = false;
        }

        m_listenFd = sockServer;

    } else {
        W_ERROR("create socket server faild! error is %s", strerror(errno));
        m_listenFd = -1;
        ret = false;
    }

    return ret;
}

int CommunicationServer::ClientObjectCount()
{
    pthread_mutex_lock(&m_operateClientObjectMapMutex);
    int ret = m_clientObjectMap.size();
    pthread_mutex_unlock(&m_operateClientObjectMapMutex);

    return ret;
}

bool CommunicationServer::addClientObject(
    const CommunicationServer::ClientObject &client)
{
    bool ret = false;
    int fd = INVALID_SOCKET;

    if (client.pEvWatcher) {
        fd = (client.pEvWatcher)->fd;
    }

    pthread_mutex_lock(&m_operateClientObjectMapMutex);
    auto iter = m_clientObjectMap.find(fd);
    if (iter == m_clientObjectMap.end()) {
        /** 不存在 **/
        m_clientObjectMap[fd] = client;
        ret = true;
    }

    pthread_mutex_unlock(&m_operateClientObjectMapMutex);

    return ret;
}

bool CommunicationServer::findClientObject(
    const int fd, CommunicationServer::ClientObject &client)
{
    bool ret = false;
    auto iter = m_clientObjectMap.find(fd);

    pthread_mutex_lock(&m_operateClientObjectMapMutex);

    if (iter != m_clientObjectMap.end()) {
        client = m_clientObjectMap[fd];
        ret = true;
    }

    pthread_mutex_unlock(&m_operateClientObjectMapMutex);

    return ret;
}

bool CommunicationServer::removeClientObjectByFd(const int fd)
{
    bool ret = true;
    auto iter = m_clientObjectMap.find(fd);

    pthread_mutex_lock(&m_operateClientObjectMapMutex);

    if (iter != m_clientObjectMap.end()) {
        /** 存在 **/
        ClientObjectDisconnect(iter->second);
        uninitClientObject(iter->second);
        m_clientObjectMap.erase(iter);
    }

    pthread_mutex_unlock(&m_operateClientObjectMapMutex);

    if (ClientObjectCount() == 0) {
        // TODO
    }

    return ret;
}

void CommunicationServer::ClientObjectDisconnect(
    const CommunicationServer::ClientObject &client)
{
    if (client.pEvWatcher) {
        int fd = (client.pEvWatcher)->fd;
        close(fd);
        ev_io_stop(m_eventloop, client.pEvWatcher);
    }
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
        W_ERROR("accept got invalid event...");
        return;
    }
    // accept连接
    newfd = accept(w->fd, (struct sockaddr *)&clientAddress, &addrlen);
    if (-1 == newfd) {
        W_ERROR("socket accept faild! error is %s", strerror(errno));
        return;
    }
    //设置非阻塞
    if (-1 == fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL) | O_NONBLOCK)) {
        close(newfd);
        return;
    }

    if (newfd > 0) {
        //加入事件循环
        ev_io_init(acceptWatcher, recvUserMsgCallback, newfd, EV_READ);
        ev_io_start(loop, acceptWatcher);

        ClientObject c;
        if (s_singletonHandle) {
            s_singletonHandle->initClientObject(c, acceptWatcher);
            s_singletonHandle->addClientObject(c);

            W_INFO("New client ip: %s Fd %d ",
                   inet_ntoa(clientAddress.sin_addr), newfd);
        }
    }

    return;
}

void CommunicationServer::recvUserMsgCallback(struct ev_loop *loop, ev_io *w,
                                              int events)
{
    (void)loop;

    char buffer[1024] = { 0 };
    if (EV_ERROR & events) {
        W_ERROR("read got invalid event...");
        return;
    }

    int socketFd = w->fd;
    bool isFindClient = true;
    int ret = 0;

    if (s_singletonHandle) {
        ClientObject client;
        isFindClient = s_singletonHandle->findClientObject(socketFd, client);
        if (isFindClient) {
            ret = read(socketFd, buffer, sizeof(buffer));

            if (ret < 0) {
                // Socket 接收出错
                W_ERROR("Socket error. code = %d", ret);
                s_singletonHandle->removeClientObjectByFd(socketFd);
            } else if (0 == ret) {
                //用户断开连接
                W_ERROR("robotUser disconnect. fd = %d", socketFd);
                s_singletonHandle->removeClientObjectByFd(socketFd);
                W_INFO("Client Count is ---------->>>>> %d",
                       s_singletonHandle->ClientObjectCount());
            } else {
                W_INFO("READ:\r\n    %s ", buffer);
            }
        }
    }

    //    int32_t bytes = read(w->fd, buffer, sizeof(buffer));
    //    if (-1 == bytes) {
    //        // tcp Error
    //        if (EINTR != errno && EAGAIN != errno) {
    //            res = 1;
    //        }
    //    } else if (0 == bytes) {
    //        // tcp Close
    //        res = 2;
    //    }

    //    if (0 != res) {
    //        //关闭事件循环并释放watcher
    //        W_INFO("TCP CLOSE ");
    //        ev_io_stop(loop, w);
    //        free(w);
    //    } else {
    //        W_INFO("READ:\r\n    %s ", buffer);

    //反序列化
    std::string recv(buffer);
    Test::Person p1;
    p1.ParseFromString(recv);
    std::cout << p1.id() << std::endl;
    std::cout << p1.name() << std::endl;
    std::cout << p1.email() << std::endl;
}
