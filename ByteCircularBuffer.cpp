#include "ByteCircularBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

ByteCircularBuffer_t *BCBCreateNew(uint16_t size)
{
    ByteCircularBuffer_t *bcb = (ByteCircularBuffer_t *)calloc(1, sizeof(ByteCircularBuffer_t));
    if (!bcb)
    {
        return nullptr;
    }
    bcb->buffer_size = size;
    bcb->buffer = (unsigned char *)calloc(size, sizeof(unsigned char));
    if (!bcb->buffer)
    {
        free(bcb);
        return nullptr;
    }
    bcb->current_size = 0;
    bcb->front = 0;
    bcb->rear = 0;
    pthread_mutex_init(&bcb->lock, NULL);

    return bcb;
}

void BCBFree(ByteCircularBuffer_t *bcb)
{
    if (!bcb)
    {
        return;
    }
    free(bcb->buffer);
    pthread_mutex_destroy(&bcb->lock);

    free(bcb); // here bcb is a copy of original pointer, BCBfree() called place, shoudl set bcb = NULL;
}

uint16_t BCBAvailableSize(ByteCircularBuffer_t *bcb)
{
    return bcb->buffer_size - bcb->current_size;
}

uint16_t BCBWrite(ByteCircularBuffer_t *bcb, const unsigned char *data, uint16_t data_size)
{
    if (!bcb || !data)
    {
        return -1;
    }
    if (data_size > (bcb->buffer_size - bcb->current_size))
    {
        return 0;
    }
    pthread_mutex_lock(&bcb->lock);
    uint16_t sapce_till_end = bcb->buffer_size - bcb->front;
    if (data_size <= sapce_till_end)
    {
        memcpy(&bcb->buffer[bcb->front], data, data_size);
    }
    else
    {
        memcpy(&bcb->buffer[bcb->front], data, sapce_till_end);
        memcpy(&bcb->buffer[0], data + sapce_till_end, data_size - sapce_till_end);
    }
    bcb->front = (bcb->front + data_size) % bcb->buffer_size;
    bcb->current_size += data_size;
    pthread_mutex_unlock(&bcb->lock);
    return data_size;
}

uint16_t BCBRead(ByteCircularBuffer_t *bcb, unsigned char *buffer, uint16_t data_size, bool remove_read_bytes)
{
    pthread_mutex_lock(&bcb->lock);
    if (data_size > bcb->current_size || !bcb || !buffer)
    {
        return 0;
    }
    uint16_t data_till_end = bcb->buffer_size - bcb->rear;
    if (data_size <= data_till_end)
    {
        memcpy(buffer, &bcb->buffer[bcb->rear], data_size);
    }
    else
    {
        memcpy(buffer, &bcb->buffer[bcb->rear], data_till_end);
        memcpy(buffer + data_till_end, &bcb->buffer[0], data_size - data_till_end);
    }

    if (remove_read_bytes)
    {
        bcb->rear = (bcb->rear + data_size) % bcb->buffer_size;
        bcb->current_size -= data_size;
    }
    pthread_mutex_unlock(&bcb->lock);
    return data_size;
}

bool BCBIsFull(ByteCircularBuffer_t *bcb)
{
    return bcb->current_size == bcb->buffer_size;
}

void BCBReset(ByteCircularBuffer_t *bcb)
{
    bcb->current_size = 0;
    bcb->front = 0;
    bcb->rear = 0;
}