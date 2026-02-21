#include "TcpNewConnectionAcceptor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include "TcpServerController.h"
#include "network_utils.h"
#include "TcpClient.h"
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"

TcpNewConnectionAcceptor::TcpNewConnectionAcceptor(TcpServerController *tcp_ctrlr)
{
    this->accept_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->accept_fd < 0)
    {
        printf("Error: Could not create Accept FD\n");
        exit(0);
    }
    this->accept_new_conn_thread = (pthread_t *)calloc(1, sizeof(pthread_t)); // allocate memory to this thread, not started,Posix thread
    this->tcp_ctrlr = tcp_ctrlr;
}

TcpNewConnectionAcceptor::~TcpNewConnectionAcceptor()
{
}

/*
   2, Create an infite loop
   3, invoke accept() to accept connections
   4, Notify the application for new connections
*/
void TcpNewConnectionAcceptor::StartTcpConnectionAcceptorThreadInternal()
{
    int opt = 1;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->tcp_ctrlr->port_no);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);
    if (setsockopt(this->accept_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("setsockopt Failed\n");
        exit(0);
    }
    if (setsockopt(this->accept_fd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("setsockopt Failed\n");
        exit(0);
    }
    /*Let's bind the socket */
    if (bind(this->accept_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Error:Acceptor socket bind failed[%s(0x%x).%d].error = %d\n", network_convert_ip_n_to_p(this->tcp_ctrlr->ip_addr, 0),
               this->tcp_ctrlr->ip_addr, this->tcp_ctrlr->port_no, errno);
        exit(0);
    }
    if (listen(this->accept_fd, 5) < 0)
    {
        printf("listen failed\n");
        exit(0);
    }
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int comm_sock_fd;

    while (true)
    {
        comm_sock_fd = accept(this->accept_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (comm_sock_fd < 0)
        {
            printf("Error Accepting New Connection\n");
            continue;
        }
        TcpClient *tcp_client = new TcpClient(client_addr.sin_addr.s_addr, client_addr.sin_port);
        tcp_client->tcp_ctrlr = this->tcp_ctrlr;
        tcp_client->comm_fd = comm_sock_fd;

        // this->tcp_ctrlr->client_connected(this->tcp_ctrlr, tcp_client); // CAS will notify application a new connection is connected
        if (this->tcp_ctrlr->client_connected)
        {
            this->tcp_ctrlr->client_connected(this->tcp_ctrlr, tcp_client);
        }
        tcp_client->msgd = new TcpMsgFixedSizeDemarcar(27);

        /*Tell the TCP Controller to further process the Client*/
        this->tcp_ctrlr->ProcessNewClient(tcp_client);

        printf("Connection Acceptted from Client[%s,%d]\n", network_convert_ip_n_to_p(htonl(client_addr.sin_addr.s_addr), 0),
               htons(client_addr.sin_port));
    }
}

static void *tcp_listen_for_connections(void *arg)
{
    TcpNewConnectionAcceptor *tcp_new_conn_acc = (TcpNewConnectionAcceptor *)arg;
    tcp_new_conn_acc->StartTcpConnectionAcceptorThreadInternal(); // 2,3,4,step will be implemented here
    return nullptr;
}

/* 1, start a thread
   2, Create an infite loop
   3, invoke accept() to accept connections
   4, Notify the application for new connections
*/
void TcpNewConnectionAcceptor::StartTcpConnectionAcceptorThread()
{
    if (pthread_create(this->accept_new_conn_thread, NULL, tcp_listen_for_connections, (void *)this))
    {
        printf("%s()Thread Creation failded.error=%d\n", __FUNCTION__, errno);
        exit(0);
    }
    printf("Service Started:TcpNewConnectionAcceptorThread\n");
}