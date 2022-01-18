#ifndef COMMUNICATIONSERVER_H
#define COMMUNICATIONSERVER_H

#include "ev.h"
#include <map>

#define INVALID_SOCKET (~0) //无效的Socket

class CommunicationServer
{
private:
    enum ConnectState
    {
        Disconnect = 0,      //未连接或已断开连接
        ConnectNotLogin = 1, //连接但未登录
        ConnectAndLogin = 2,
    };

    typedef struct
    {
        ev_io *pEvWatcher; //事件IO监视器
        char *pRecvBuffer; //接收消息缓冲区
        char *pSendBuffer; //发送消息缓冲区

        ConnectState connectState; //连接状态
        time_t connectTimestamp;   //连接时间戳
        bool isReserve;            //预留
        pthread_mutex_t sendMutex;

    } ClientObject;

    enum
    {
        RECV_BUFFER_SIZE = 2 * 1024 * 1024
    }; //接收缓冲区的大小
    enum
    {
        SEND_BUFFER_SIZE = RECV_BUFFER_SIZE - 256
    }; //发送缓冲区的大小

    enum
    {
        SIGNLE_PACKAGE_MESSAGE_LEN = 1 * 1024
    }; //单包数据的最大

public:
    CommunicationServer();

    bool serviceLoopStart(void);

private:
    void init();
    void initClientObject(ClientObject &client, ev_io *pWatcher);
    void uninitClientObject(ClientObject &client);

private:
    bool initSocket(void);

protected:
    int ClientObjectCount();
    bool addClientObject(const ClientObject &client);
    bool findClientObject(const int fd, ClientObject &client);
    bool removeClientObjectByFd(const int fd);
    void ClientObjectDisconnect(const ClientObject &client);

private:
    //接收新用户连接的回调函数
    static void acceptNewUserCallback(struct ev_loop *loop, ev_io *w,
                                      int events);
    static void recvUserMsgCallback(struct ev_loop *loop, ev_io *w, int events);

private:
    int m_listenFd;
    struct ev_loop *m_eventloop{ nullptr }; // libev事件循环
    ev_io m_connectIoWatcher;               //监控新客户连接
    std::map<int, ClientObject> m_clientObjectMap;

    pthread_mutex_t
        m_operateClientObjectMapMutex; //防止同时操作客户列表的互斥锁

private:
    static CommunicationServer *s_singletonHandle; //单例指针
};

#endif // COMMUNICATIONSERVER_H
