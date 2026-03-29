#ifndef __TCPSERVER__
#define __TCPSERVER__

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <list>
#include "TcpMsgDemarcar.h"

#define TCP_SERVER_INITIALIZED (1)
#define TCP_SERVER_RUNNING (2)
#define TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS (4)
#define TCP_SERVER_NOT_LISTENING_CLIENTS (8)
#define TCP_SERVER_CREATE_MULTI_THREAD_CLIENT (16)

class TcpNewConnectionAcceptor;
class TcpClientServiceManager;
class TcpClientDbManager;
class TcpClient;

class TcpServerController
{
private:
    TcpNewConnectionAcceptor *tcp_new_conn_acc;
    TcpClientDbManager *tcp_client_db_mgr;
    TcpClientServiceManager *tcp_client_svc_mgr;
    uint32_t state_flags = 0;

public:
    uint32_t ip_addr;
    uint16_t port_no;
    std::string name;

    void (*client_connected)(const TcpServerController *, const TcpClient *);
    void (*client_disconected)(const TcpServerController *, const TcpClient *);
    void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t);

    void SetServerNotifCallbacks(void (*client_connected)(const TcpServerController *, const TcpClient *),
                                 void (*client_disconected)(const TcpServerController *, const TcpClient *),
                                 void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t));

    /*Constructor and Destructor*/
    TcpServerController(std::string ip_addr, uint16_t port_no, std::string name); // Ip address, port number, name of TCP server  name
    ~TcpServerController();
    void Start();
    void Stop();
    void ProcessNewClient(TcpClient *);
    void Display();
    void SetBit(uint32_t bit_value);
    void UnSetBit(uint32_t bit_value);
    bool IsBitSet(uint32_t bit_value);
    void StopConnectionAcceptingSvc();
    void StartConectionAcceptionSvc();
    /*Listen for Connected Clients*/
    void StopClientSvcMgr();
    void StartClientSvcMgr();
    void CopyAllClientsTolist(std::list<TcpClient *> *list);
    void CreateActiveAClient(uint32_t server_ip_addr, uint16_t server_port_no);
    TcpClientServiceManager *GetClientServiceManger();
    TcpNewConnectionAcceptor *GetNewAccptionManager();
};

#endif
