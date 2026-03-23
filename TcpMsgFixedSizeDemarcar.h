#ifndef __TCP_DEMARCAR_FIXED_SIZE__
#define __TCP_DEMARCAR_FIXED_SIZE__
#include <stdint.h>
#include "TcpClient.h"

class TcpMsgFixedSizeDemarcar : public TcpMsgDemarcar
{
private:
    uint16_t msg_fixed_size;

public:
    bool IsBufferReadyToFlush() override;
    void ProcessClientMsg(TcpClient *) override;

    /*Constructor*/
    TcpMsgFixedSizeDemarcar(uint16_t fixed_size);

    /*Destructor*/
    ~TcpMsgFixedSizeDemarcar();
    void Destroy();
};

#endif