#ifndef __TCP_DEMARCAR_VARIABLE_SIZE__
#define __TCP_DEMARCAR_VARIABLE_SIZE__
#include <stdint.h>
#include "TcpClient.h"
#include "TcpMsgDemarcar.h"

#define HDR_MSG_SIZE 2
class TcpMsgVariabSizeDemarcar : public TcpMsgDemarcar
{

public:
    TcpMsgVariabSizeDemarcar();
    ~TcpMsgVariabSizeDemarcar();
    bool IsBufferReadyToFlush() override;
    void ProcessClientMsg(TcpClient *) override;
    void Destroy();
};

#endif