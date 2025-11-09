#ifndef __TcpClientDbManager__
#define __TcpClientDbManager__

#include <list>
#include <semaphore.h>
#include <stdint.h>
#include <pthread.h>
#include <vector>
#include "TcpClient.h"
class TcpClient;
class TcpServerController;

class TcpClientDbManager
{

private:
    pthread_rwlock_t rwlock;
    std::list<TcpClient *> tcp_client_db;

public:
    TcpServerController *tcp_ctrlr;
    TcpClientDbManager(TcpServerController *);
    ~TcpClientDbManager();

    void StartTcpClientDbMgrInit();

    /*Client DB mgmt functions*/
    void Purge();
    void AddClienttoDb(TcpClient *tcp_client);
    void DisplayClientDb();
};

#endif