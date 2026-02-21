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
void TcpMsgDemarcar::ProcessMsg(TcpClient *tcp_client, unsigned char *msg_recvd, uint16_t msg_size)
{
    // assert(BCBWrite(tcp_client->msgd->bcb, msg_recvd, msg_size));
    bool ok = BCBWrite(tcp_client->msgd->bcb, msg_recvd, msg_size);
    if (!ok)
    {
        fprintf(stderr,
                "BCBWrite FAILED: msg_size=%u buffer_size=%u current=%u front=%u rear=%u\n",
                msg_size, this->bcb->buffer_size, this->bcb->current_size, this->bcb->front, this->bcb->rear);
    }

    if (this->bcb->current_size > this->bcb->buffer_size)
    {
        fprintf(stderr, "BCB CORRUPT: current_size > buffer_size\n");
    }
    if (this->bcb->front >= this->bcb->buffer_size || this->bcb->rear >= this->bcb->buffer_size)
    {
        fprintf(stderr, "BCB CORRUPT: front/rear out of range\n");
    }
    if (!this->IsBufferReadyToFlush())
    {
        return;
    }
    this->ProcessClientMsg(tcp_client);
}