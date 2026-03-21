#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "ByteCircularBuffer.h" //need to be added later
#include "TcpClient.h"

TcpMsgDemarcar::TcpMsgDemarcar(uint16_t circular_buffer_len)
{
    this->bcb = BCBCreateNew(circular_buffer_len);
    this->buffer = (unsigned char *)calloc(MAX_CLIENT_BUFFER_SIZE, sizeof(unsigned char));
}
TcpMsgDemarcar::TcpMsgDemarcar()
{
    this->bcb = BCBCreateNew(DEFAULT_CBC_SIZE);
    this->buffer = (unsigned char *)calloc(MAX_CLIENT_BUFFER_SIZE, sizeof(unsigned char));
    printf("BCB created: requested=%u actual =%u\n", this->bcb->buffer_size, this->bcb->current_size);
}

TcpMsgDemarcar::~TcpMsgDemarcar()
{
    assert(!this->bcb);
    assert(!this->buffer);
}

void TcpMsgDemarcar::Destroy()
{
    if (this->bcb)
    {
        BCBFree(this->bcb);
        this->bcb = NULL;
    }
    if (this->buffer)
    {
        free(this->buffer);
        this->buffer = NULL;
    }
}
void TcpMsgDemarcar::ProcessMsg(TcpClient *tcp_client, const unsigned char *msg_recvd, uint16_t msg_size)
{
    uint16_t written = BCBWrite(this->bcb, msg_recvd, msg_size);
    // assert(BCBWrite(this->bcb, msg_recvd, msg_size));
    if (written != msg_size)
    {
        printf("RingBuffer Full! dropping data. "
               "current_size =%u capacity=%u incoming=%u\n",
               this->bcb
                   ->current_size,
               this->bcb->buffer_size, msg_size);
        return;
    }
    this->ProcessClientMsg(tcp_client);
}
