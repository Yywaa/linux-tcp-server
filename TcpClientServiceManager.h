#ifndef __TcpClientServiceManager__
#define __TcpClientServiceManager__

// class TcpServerController;
#include "TcpServerController.h"
#include <sys/epoll.h>

#define EPOLL 1
class TcpClient;
class TcpNewConnectionAcceptor;

class TcpClientServiceManager
{
public:
    int max_fd;
    int epfd;
    fd_set active_fd_set;
    fd_set backup_fd_set;
    pthread_t *client_svc_mgr_thread;
    std::list<TcpClient *> tcp_client_db;
    int GetMaxFd();
    void CopyClientFDtoFDSet(fd_set *fdset);

    /*
    pthread_mutex_t ready_mutex;
    pthread_cond_t ready_cond;
    bool is_ready;*/

public:
    TcpServerController *tcp_ctrlr;
    TcpClientServiceManager(TcpServerController *);
    ~TcpClientServiceManager();
    void StartTcpClientServiceManagerThread();
    void ClientFDStartListen(TcpClient *tcp_client);
    void StartTcpClientServiceManagerThreadInternal();
    void StopTcpClientServiceManagerThread();
    void AddClientToDB(TcpClient *);
    void Stop();
    TcpClient *GetClientByFd(int fd);
    void AddClientToEpoll(TcpClient *client);
    // void WaitUntilReady();
    void RemoveClient(TcpClient *client);
};

#endif