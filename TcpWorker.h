#ifndef __TCP_WORKER__
#define __TCP_WORKER__

#include <iostream>
#include <list>
#include <sys/epoll.h>
#include <unordered_map>
class TcpClient;
class TcpServerController;

class TcpWorker
{
public:
    TcpWorker(TcpServerController *ctrl, int id);

    int worker_id; // id of each worker
    int epfd;
    pthread_t thread;
    std::list<TcpClient *> clients;
    std::unordered_map<int, TcpClient *> fd_map;
    TcpServerController *Tcp_ctrl;
    void Start();
    void AddClient(TcpClient *client);
    static void *WorkerThreadFn(void *arg);
};

#endif