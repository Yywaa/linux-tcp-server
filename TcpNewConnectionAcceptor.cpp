#include <stdio.h>
#include <unistd.h>
#include "TcpNewConnectionAcceptor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include "TcpServerController.h"
#include "network_utils.h"
#include "TcpClient.h"
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"
#include "TcpMsgVariabSizeDemarcar.h"
#include "TcpClientServiceManager.h"

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

void TcpNewConnectionAcceptor::StopTcpNewConnectionAcceptorThread()
{
    if (!this->accept_new_conn_thread)
    {
        return;
    }
    pthread_cancel(*this->accept_new_conn_thread);
    /*wait until the thread is cancelled successfully*/
    pthread_join(*this->accept_new_conn_thread, NULL);
    free(this->accept_new_conn_thread);
    this->accept_new_conn_thread = NULL;
}

void TcpNewConnectionAcceptor::Stop()
{
    // 1,Stop the CAS thread if running
    // 2,Release the resource (accept id)
    // 3,delete this instance of CAS
    this->StopTcpNewConnectionAcceptorThread();
    close(this->accept_fd);
    this->accept_fd = 0;
    delete this;
}

int TcpNewConnectionAcceptor::GetAcceptFd()
{
    return this->accept_fd;
}