#include "TcpClient.h"
#include "network_utils.h"
#include <stdio.h>
#include <arpa/inet.h>

TcpClient::TcpClient(uint32_t ip_addr, uint16_t port_no)
{
    this->ip_addr = ip_addr;
    this->port_no = port_no;
    this->state_flags = 0;
}

void TcpClient::Display()
{
    char ip_addr_str1[16];
    char ip_addr_str2[16];
    printf("Tcp client:[%s, %d] connected to [%s, %d]\n", network_convert_ip_n_to_p(this->ip_addr, ip_addr_str1), (this->port_no), network_convert_ip_n_to_p(this->server_ip_addr, ip_addr_str2), (this->server_port_no));
    printf("Flags:\n");
    if (this->IsStateSet(TCP_CLIENT_STATE_ACTIVE_OPENER))
    {
        printf("TCP_CLIENT_STATE_ACTIVE_OPENER\n");
    }
    if (this->IsStateSet(TCP_CLIENT_STATE_CONNECT_IN_PROGRESS))
    {
        printf("TCP_CLIENT_STATE_CONNECT_IN_PROGRESS\n");
    }
    if (this->IsStateSet(TCP_CLIENT_STATE_CONNECTED))
    {
        printf("TCP_CLIENT_STATE_CONNECTED\n");
    }
    if (this->IsStateSet(TCP_CLIENT_STATE_MULTIPLEX_LISTEN))
    {
        printf("TCP_CLIENT_STATE_MULTIPLEX_LISTEN\n");
    }
    if (this->IsStateSet(TCP_CLIENT_STATE_PASSIVE_OPENER))
    {
        printf("TCP_CLIENT_STATE_PASSIVE_OPENER\n");
    }
    if (this->IsStateSet(TCP_CLIENT_STATE_THREADED))
    {
        printf("TCP_CLIENT_STATE_THREADED\n");
    }
}

void TcpClient::Abort()
{
    if (this->comm_fd)
    {
        close(this->comm_fd);
        this->comm_fd = 0;
    }
    this->tcp_ctrlr = NULL;
    if (this->msgd)
    {
        this->msgd->Destroy();
        delete this->msgd;
        this->msgd = NULL;
    }
    delete this;
}

void TcpClient::SetState(client_state_bit state_flags)
{
    this->state_flags |= state_flags;
}

void TcpClient::UnSetState(client_state_bit state_flags)
{
    this->state_flags &= ~state_flags;
}

bool TcpClient::IsStateSet(client_state_bit state_flags)
{
    return (this->state_flags & state_flags);
}