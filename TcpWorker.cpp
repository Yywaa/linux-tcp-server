#include "TcpWorker.h"
#include "TcpServerController.h"
#include "TcpClient.h"
#include "ByteCircularBuffer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>

#define FD_MAP_TEST 1

TcpWorker::TcpWorker(TcpServerController *ctrl, int id)
{
    this->worker_id = id;
    this->Tcp_ctrl = ctrl;
}

void TcpWorker::Start()
{
    epfd = epoll_create1(0);
    pthread_create(&thread, NULL, WorkerThreadFn, this);
}

void TcpWorker::AddClient(TcpClient *client)
{
    printf("[Worker %d] New client fd=%d assigned\n", this->worker_id, client->comm_fd);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client->comm_fd;

    epoll_ctl(this->epfd, EPOLL_CTL_ADD, client->comm_fd, &ev);

#if FD_MAP_TEST
    fd_map[client->comm_fd] = client;
#elif
    clients.push_back(client);

#endif
}

void *TcpWorker::WorkerThreadFn(void *arg)
{
    unsigned char client_recv_buffer[MAX_CLIENT_BUFFER_SIZE];
    TcpWorker *worker = (TcpWorker *)arg;

    struct epoll_event events[64];

    while (true)
    {
        int nfds = epoll_wait(worker->epfd, events, 64, -1);

        for (int i = 0; i < nfds; i++)
        {
            int fd = events[i].data.fd;
            TcpClient *client = nullptr;

            // search client
#if FD_MAP_TEST
            auto start = std::chrono::high_resolution_clock::now();
            auto it = worker->fd_map.find(fd);
            if (it == worker->fd_map.end())
            {
                continue;
            }
            client = it->second;
            auto end = std::chrono::high_resolution_clock::now();
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            printf("lookup cost =%d ns\n", ns);

#else
            auto start = std::chrono::high_resolution_clock::now();
            for (auto c : worker->clients)
            {
                if (c->comm_fd == fd)
                {
                    client = c;
                    break;
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            printf("lookup cost =%d ns\n", ns);
#endif

            if (!client)
            {
                continue;
            }
            while (1)
            {
                uint16_t space = BCBAvailableSize(client->msgd->bcb);
                if (space == 0)
                {
                    printf("[Worker] Backpressure\n");
                    break;
                }
                int bytes = recv(fd, client_recv_buffer, space, 0);
                /*
                                pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
                                pthread_mutex_lock(&log_mutex);
                                // printf("[Worker %d] recv %d bytes from fd %d\n", worker->worker_id, bytes, fd);
                                // printf("[Worker %d | Thread %p] recv %d bytes from fd %d\n", worker->worker_id, (void *)pthread_self(), bytes, fd); // worker 0 worker1 will disrupt printf, printf is not atomic
                                pthread_mutex_unlock(&log_mutex);
                */
                if (bytes > 0)
                {
                    client->msgd->ProcessMsg(client, client_recv_buffer, bytes);
                }
                else if (bytes == 0)
                {
                    printf("[Worker] client closed\n");
                    close(fd);
                    epoll_ctl(worker->epfd, EPOLL_CTL_DEL, fd, NULL);

                    worker->clients.remove(client);
                    delete client;
                    client = nullptr;
                    break;
                }
                else
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        break;
                    }
                    perror("recv error");
                    close(fd);
                    epoll_ctl(worker->epfd, EPOLL_CTL_DEL, fd, NULL);
#if FD_MAP_TEST
                    worker->fd_map.erase(fd);
                    delete client;
#elif
                    worker->clients.remove(client);
                    delete client;

#endif

                    client = nullptr;
                    break;
                }
            }
        }
    }
}