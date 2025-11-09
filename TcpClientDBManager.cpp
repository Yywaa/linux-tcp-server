#include "TcpServerController.h"
#include "TcpClientDBManager.h"

TcpClientDbManager::TcpClientDbManager(TcpServerController *tcp_ctrlr)
{

    this->tcp_ctrlr = tcp_ctrlr;
}

TcpClientDbManager::~TcpClientDbManager()
{
}

void TcpClientDbManager::StartTcpClientDbMgrInit()
{
}

// add new client to data base
void TcpClientDbManager::AddClienttoDb(TcpClient *tcp_client)
{
    // this->tcp_client_db.emplace_back(tcp_client);
    tcp_client_db.emplace_back(tcp_client);
    printf("new client added to DB manager\n");
}

void TcpClientDbManager::DisplayClientDb()
{
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it)
    {
        tcp_client = *it;
        tcp_client->Display();
    }
}