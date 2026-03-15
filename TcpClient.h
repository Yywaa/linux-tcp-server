#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <unistd.h>
#include <stdint.h>
#include "TcpServerController.h"

/* TCP Client States */

#define TCP_CLIENT_STATE_CONNECT_IN_PROGRESS 1 // when connect() is in progress but not yet successful.

#define TCP_CLIENT_STATE_CONNECTED 2 // when connect () Or accept() is successful

#define TCP_CLIENT_STATE_PASSIVE_OPENER 8 // when TcpClient object is created due to accept()

#define TCP_CLIENT_STATE_ACTIVE_OPENER 16 // when TcpClient object is created due to connect()

#define TCP_CLIENT_STATE_MULTIPLEX_LISTEN 32 // When TcpClient is serviced by select() to recv data

#define TCP_CLIENT_STATE_THREADED 64 // When TcpClient has its own separate thread to recv data using recvfrom()
typedef uint32_t client_state_bit;
class TcpServerController;
class TcpMsgDemarcar;

#define MAX_CLIENT_BUFFER_SIZE 8192
class TcpClient
{
private:
public:
    uint32_t ip_addr;
    uint16_t port_no;
    uint32_t server_ip_addr;
    uint32_t server_port_no;
    client_state_bit state_flags;
    int comm_fd;
    TcpServerController *tcp_ctrlr;
    TcpMsgDemarcar *msgd;
    TcpClient(uint32_t ip_addr, uint16_t port_no);
    void Display();
    void Abort();

    void SetState(client_state_bit flag_bit);
    void UnSetState(client_state_bit flag_bit);
    bool IsStateSet(client_state_bit flag_bit);
};

#endif