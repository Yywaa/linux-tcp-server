#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <cassert>
#include "TcpServerController.h"
#include "TcpClientServiceManager.h"
#include "TcpClient.h"
#include "TcpMsgVariabSizeDemarcar.h"
#include "ByteCircularBuffer.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpWorker.h"

TcpClientServiceManager::TcpClientServiceManager(TcpServerController *tcp_ctrlr)
{
    this->tcp_ctrlr = tcp_ctrlr;
    this->max_fd = 0;
    FD_ZERO(&this->active_fd_set);
    FD_ZERO(&this->backup_fd_set);
    this->client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
}

TcpClientServiceManager::~TcpClientServiceManager()
{
}

void TcpClientServiceManager::StartTcpClientServiceManagerThreadInternal()
{

    unsigned char client_recv_buffer[MAX_CLIENT_BUFFER_SIZE];
    // create epoll
    this->epfd = epoll_create1(0);
    struct epoll_event ev, events[64];

    // move bind listen to service thread here
    int opt = 1;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->tcp_ctrlr->port_no);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);
    // int listen_fd = this->tcp_ctrlr->GetNewAccptionManager()->GetAcceptFd();
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("Socket");
        exit(1);
    }
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_fd, 5);

    // non-blocking
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);
    // reactor below

    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;

    epoll_ctl(this->epfd, EPOLL_CTL_ADD, listen_fd, &ev);
    // reactor abolve

    /*Invoke select system call on all Clients present in Client DB*/
    int rcv_bytes;
    TcpClient *tcp_client, *next_tcp_client;
    struct sockaddr_in client_addr;
    std::list<TcpClient *>::iterator it;

    this->max_fd = this->GetMaxFd();

#if EPOLL
    for (auto tcp_client : this->tcp_client_db)
    {
        ev.events = EPOLLIN;
        ev.data.fd = tcp_client->comm_fd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, tcp_client->comm_fd, &ev);
    }
#elif
    FD_ZERO(&this->backup_fd_set);
    this->CopyClientFDtoFDSet(&this->backup_fd_set);
#endif

    while (true)
    {
#if EPOLL
        int nfds = epoll_wait(epfd, events, 64, -1);
        for (int i = 0; i < nfds; i++)
        {
            int fd = events[i].data.fd;
            if (fd == listen_fd)
            {
                printf("EPOLL:listen_fd triggered\n");
                while (1)
                {
                    socklen_t addr_len = sizeof(client_addr);
                    int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);
                    if (client_fd < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            break;
                        }
                        perror("accept error");
                        break;
                    }
                    // set non-blocking
                    int flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                    // create tcp client
                    TcpClient *tcp_client = new TcpClient(client_addr.sin_addr.s_addr, client_addr.sin_port);
                    tcp_client->comm_fd = client_fd;
                    tcp_client->tcp_ctrlr = this->tcp_ctrlr;
#if FIX_SZIE_DEMAR
                    tcp_client->msgd = new TcpMsgFixedSizeDemarcar(27);
#else
                    tcp_client->msgd = new TcpMsgVariabSizeDemarcar();
#endif
                    // add tcp client ot DB
                    // this->AddClientToDB(tcp_client);
                    // add to epoll
                    struct epoll_event client_ev;
                    client_ev.events = EPOLLIN;
                    client_ev.data.fd = client_fd;
                    // epoll_ctl(this->epfd, EPOLL_CTL_ADD, client_fd, &client_ev);
                    TcpWorker *worker = this->tcp_ctrlr->GetNextWorker();
                    printf("Assign fd=%d to worker %d\n", client_fd, worker->worker_id);
                    worker->AddClient(tcp_client);

                    printf("New client accepted [%d]\n", client_fd);
                    printf("New client -> worker\n");
                }
                continue;
            }
        }
    }
}

#elif
        memcpy(&this->active_fd_set, &this->backup_fd_set, sizeof(fd_set));
        select(this->max_fd + 1, &this->active_fd_set, 0, 0, 0);

        for (it = this->tcp_client_db.begin(), tcp_client = *it; it != this->tcp_client_db.end(); tcp_client = next_tcp_client)
        {
            // next_tcp_client = *(++it);
            if (FD_ISSET(tcp_client->comm_fd, &this->active_fd_set))
            {
                // rcv_bytes = recvfrom(tcp_client->comm_fd, client_recv_buffer, MAX_CLIENT_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

                if (tcp_client->msgd)
                {
                    uint16_t space = BCBAvailableSize(tcp_client->msgd->bcb);
                    if (space == 0)
                    {
                        printf("BackPressure: Buffer full, skip recv\n");
                        next_tcp_client = *(++it);
                        continue; // skip current tcp client;
                    }
                    rcv_bytes = recvfrom(tcp_client->comm_fd, client_recv_buffer, space, 0, (struct sockaddr *)&client_addr, &addr_len);
                    if (rcv_bytes == 0)
                    {
                        /*from gpt*/
                        printf("Client closed connection\n");

                        close(tcp_client->comm_fd);
                        FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
                        it = this->tcp_client_db.erase(it);
                        continue;
                        /*original*/
                        /*
                        printf("error = %dn", errno);
                        sleep(1);*/
                    }
                    tcp_client->msgd->ProcessMsg(tcp_client, client_recv_buffer, rcv_bytes);
                }
                else if (this->tcp_ctrlr->client_msg_recvd)
                {
                    this->tcp_ctrlr->client_msg_recvd(this->tcp_ctrlr, tcp_client, client_recv_buffer, rcv_bytes);
                }
            }
            next_tcp_client = *(++it);
        }
#endif

void *tcp_client_svc_manager_thread_fn(void *arg)
{
    TcpClientServiceManager *svc_mgr = (TcpClientServiceManager *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    svc_mgr->StartTcpClientServiceManagerThreadInternal();
    return NULL;
}
void TcpClientServiceManager::StartTcpClientServiceManagerThread()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(this->client_svc_mgr_thread, &attr,
                   tcp_client_svc_manager_thread_fn, (void *)this);
    printf("Service started:TcpClientServiceManagerThread\n");
}

void TcpClientServiceManager::StopTcpClientServiceManagerThread()
{
    pthread_cancel(*this->client_svc_mgr_thread);
    pthread_join(*this->client_svc_mgr_thread, NULL);
    free(this->client_svc_mgr_thread);
    this->client_svc_mgr_thread = NULL;
    // printf("Service stopped:TcpClientServiceManagerThread\n");
}
int TcpClientServiceManager::GetMaxFd()
{
    int max_fd = 0;
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); it++)
    {
        tcp_client = *it; // Can I use (*it)->comm_fd here directly?yes, but  not readibility
        if (tcp_client->comm_fd > max_fd)
        {
            max_fd = tcp_client->comm_fd;
        }
    }
    return max_fd;
}
void TcpClientServiceManager::CopyClientFDtoFDSet(fd_set *fdset)
{
    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it)
    {
        tcp_client = *it;
        FD_SET(tcp_client->comm_fd, fdset);
    }
}
void TcpClientServiceManager::AddClientToDB(TcpClient *tcp_client)
{
    this->tcp_client_db.push_back(tcp_client);
    // printf("new client added to CAS data base\n");
}
void TcpClientServiceManager::ClientFDStartListen(TcpClient *tcp_client)
{
    // this->StopTcpClientServiceManagerThread();
    // printf("CLient Svc Mgr Thread is cancelled\n");

    this->AddClientToDB(tcp_client);

    // this->client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    // this->StartTcpClientServiceManagerThread();
}

void TcpClientServiceManager::Stop()
{
    this->StopTcpClientServiceManagerThread();
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client, *next_tcp_client;

    /*this assumes Svc mgr thread is already cancelled, no need to lock anaything*/
    assert(this->client_svc_mgr_thread == NULL);
    for (it = this->tcp_client_db.begin(), tcp_client = *it; it != this->tcp_client_db.end(); tcp_client = next_tcp_client)
    {
        next_tcp_client = *(++it);
        this->tcp_client_db.remove(tcp_client);
    }
    delete this;
}

TcpClient *TcpClientServiceManager::GetClientByFd(int fd)
{
    for (auto c : this->tcp_client_db)
    {
        if (c->comm_fd == fd)
        {
            return c;
        }
    }
    return nullptr;
}

void TcpClientServiceManager::AddClientToEpoll(TcpClient *client)
{
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client->comm_fd;

    epoll_ctl(this->epfd, EPOLL_CTL_ADD, client->comm_fd, &ev);
}

void TcpClientServiceManager::RemoveClient(TcpClient *client)
{
    this->tcp_client_db.remove(client);
    delete client;
}