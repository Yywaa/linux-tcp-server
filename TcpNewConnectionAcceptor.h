#ifndef __TcpNewConnectionAcceptor__
#define __TcpNewConnectionAcceptor__

#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
class TcpServerController;
class TcpNewConnectionAcceptor;
class TcpClientServiceManager;
class TcpClient;

/*New connection are accepted by using 'accept()'syscall
  We need to create a 'accept_fd'using socket()
*/

class TcpNewConnectionAcceptor
{
private:
    int accept_fd;
    pthread_t *accept_new_conn_thread; // thread obejct,this new connetion service is a thread

public:
    TcpServerController *tcp_ctrlr;

    TcpNewConnectionAcceptor(TcpServerController *);
    ~TcpNewConnectionAcceptor();
    void StartTcpConnectionAcceptorThread();
    void StartTcpConnectionAcceptorThreadInternal();

    void Stop();
    void StopTcpNewConnectionAcceptorThread();
};

#endif