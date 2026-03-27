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
    // printf("Buffer size = %u\n", this->bcb->current_size);
    uint16_t remaining = msg_size;
    const unsigned char *ptr = msg_recvd;
    while (remaining > 0)
    {
        uint16_t space = BCBAvailableSize(this->bcb);
        if (space == 0)
        {
            printf("Ring Buffer Full! dropping incoming data =%u\n", remaining);
            return;
        }
        uint16_t to_write = (remaining < space) ? remaining : space;
        uint16_t written = BCBWrite(this->bcb, ptr, to_write);
        if (written == 0)
        {
            printf("BCBwrite failed unexpectedly\n");
        }
        ptr += written;
        remaining -= written;
    }

    this->ProcessClientMsg(tcp_client);
}

bool TcpMsgDemarcar::CanAcceptMoreData()
{
    return BCBAvailableSize(this->bcb) > 0;
}