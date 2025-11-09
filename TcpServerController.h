#ifndef __TCPSERVER__
#define __TCPSERVER__

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <list>
#include "TcpMsgDemarcar.h"

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
};

#endif
