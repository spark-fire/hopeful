#ifndef COMMUNICATIONSERVER_H
#define COMMUNICATIONSERVER_H

#include "ev.h"

class CommunicationServer
{
public:
    CommunicationServer();

    bool serviceLoopStart(void);

private:
    bool initSocket(void);

private:
    //接收新用户连接的回调函数
    static void acceptNewUserCallback(struct ev_loop *loop, ev_io *w,
                                      int events);
    static void recvUserMsgCallback(struct ev_loop *loop, ev_io *w, int events);

private:
    int m_listenFd;
    struct ev_loop *m_eventloop{ nullptr }; // libev事件循环
    ev_io m_connectIoWatcher;               //监控新客户连接
};

#endif // COMMUNICATIONSERVER_H
