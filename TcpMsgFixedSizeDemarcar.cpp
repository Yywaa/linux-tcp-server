#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"
#include "TcpClient.h"
#include "TcpServerController.h"
#include "ByteCircularBuffer.h"

// the only way to initilize call the base constructor in the member initializer list: is this only way for this constructor
TcpMsgFixedSizeDemarcar::TcpMsgFixedSizeDemarcar(uint16_t msg_fixed_size) : TcpMsgDemarcar(DEFAULT_CBC_SIZE),
                                                                            msg_fixed_size(msg_fixed_size) {}

TcpMsgFixedSizeDemarcar::~TcpMsgFixedSizeDemarcar()
{
}
bool TcpMsgFixedSizeDemarcar::IsBufferReadyToFlush()
{
    if (this->bcb->current_size / this->msg_fixed_size > 0)
    {
        return true;
    }
    return false;
}
void TcpMsgFixedSizeDemarcar::Destroy()
{
    // this->Destroy(); //this will cause infinite recursion, it will call itsself forever
    // this->TcpMsgDemarcar::Destroy();
    TcpMsgDemarcar::Destroy();
}
void TcpMsgFixedSizeDemarcar::ProcessClientMsg(TcpClient *tcp_client)
{
    uint16_t bytes_read;
    if (!this->IsBufferReadyToFlush())
    {
        return;
    }
    while (bytes_read = BCBRead(this->bcb, this->buffer, this->msg_fixed_size, true))
    {
        // printf("bytes_read = %u\n", bytes_read);
        this->buffer[bytes_read] = '\0';
        tcp_client->tcp_ctrlr->client_msg_recvd(tcp_client->tcp_ctrlr, tcp_client, this->buffer, bytes_read);
    }
}