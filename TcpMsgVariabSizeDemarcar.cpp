#include "TcpMsgVariabSizeDemarcar.h"
#include "TcpClient.h"
#include "TcpServerController.h"
#include "ByteCircularBuffer.h"
#include <netinet/in.h>
TcpMsgVariabSizeDemarcar::TcpMsgVariabSizeDemarcar() : TcpMsgDemarcar(DEFAULT_CBC_SIZE)
{
}

TcpMsgVariabSizeDemarcar::~TcpMsgVariabSizeDemarcar()
{
}

bool TcpMsgVariabSizeDemarcar::IsBufferReadyToFlush()
{
    if (this->bcb->current_size <= HDR_MSG_SIZE)
    {
        return false;
    }
    uint16_t msg_len_net;
    BCBRead(this->bcb, (unsigned char *)&msg_len_net, sizeof(uint16_t), false);
    uint16_t msg_len = ntohs(msg_len_net);
    if (this->bcb->current_size < sizeof(uint16_t) + msg_len_net)
    {
        return false;
    }
    return true;
}

void TcpMsgVariabSizeDemarcar::ProcessClientMsg(TcpClient *tcp_client)
{
    uint16_t msg_len;
    while (this->IsBufferReadyToFlush())
    {
        // read hear and remove header
        BCBRead(this->bcb, (unsigned char *)&msg_len, HDR_MSG_SIZE, true);

        // read payload and remove payload
        uint16_t bytes_read = BCBRead(this->bcb, this->buffer, msg_len, true);
        this->buffer[bytes_read] = '\0';
        tcp_client->tcp_ctrlr->client_msg_recvd(tcp_client->tcp_ctrlr, tcp_client, this->buffer, bytes_read);
    }
}

void TcpMsgVariabSizeDemarcar::Destroy()
{
    TcpMsgDemarcar::Destroy();
}