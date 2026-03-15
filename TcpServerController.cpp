#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "TcpServerController.h"
#include "TcpClientDBManager.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClientServiceManager.h"
#include "network_utils.h"

TcpServerController::TcpServerController(std::string ip_addr, uint16_t port_no, std::string name)
{
  this->ip_addr = network_convert_ip_p_to_n(ip_addr.c_str());
  this->port_no = port_no;
  this->name = name;
  this->tcp_new_conn_acc = new TcpNewConnectionAcceptor(this);
  this->tcp_client_db_mgr = new TcpClientDbManager(this);
  this->tcp_client_svc_mgr = new TcpClientServiceManager(this);

  this->SetBit(TCP_SERVER_INITIALIZED);
}
TcpServerController::~TcpServerController()
{
  assert(!this->tcp_client_db_mgr);
  assert(!this->tcp_client_svc_mgr);
  assert(!this->tcp_new_conn_acc);
}

void TcpServerController::Start()
{

  /*Start CRS thread
    Start the DRS threa
    initialize the DBMS*/
  this->tcp_new_conn_acc->StartTcpConnectionAcceptorThread();
  this->tcp_client_svc_mgr->StartTcpClientServiceManagerThread();
  this->tcp_client_db_mgr->StartTcpClientDbMgrInit();
  this->SetBit(TCP_SERVER_RUNNING);

  printf("Tcp Server is Up and Running[%s,%d]\nOk.\n", network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);
}

void TcpServerController::ProcessNewClient(TcpClient *tcp_client)
{
  this->tcp_client_db_mgr->AddClienttoDb(tcp_client);
  tcp_client->SetState(TCP_CLIENT_STATE_MULTIPLEX_LISTEN);
  this->tcp_client_svc_mgr->ClientFDStartListen(tcp_client);
}

void TcpServerController::SetServerNotifCallbacks(void (*client_connected)(const TcpServerController *, const TcpClient *),
                                                  void (*client_disconected)(const TcpServerController *, const TcpClient *),
                                                  void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t))
{
  this->client_connected = client_connected;
  this->client_disconected = client_disconected;
  this->client_msg_recvd = client_msg_recvd;
}

void TcpServerController::Display()
{
  printf("Server Name: %s\n", this->name.c_str());
  printf("Listening on :[%s, %d]\n", network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);
  if (!this->IsBitSet(TCP_SERVER_RUNNING))
  {
    printf("Tcp Server Not Running\n");
    // printf("Server states when Not running: %d\n", this->state_flags);
    return;
  }
  // printf("Listening on: [%s, %d]\n", network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);
  // printf("Server states when running: %d\n", this->state_flags);
  printf("Flags: ");

  if (this->IsBitSet(TCP_SERVER_INITIALIZED))
  {
    printf("I");
  }
  else
  {
    printf("Un-I");
  }
  if (this->IsBitSet(TCP_SERVER_RUNNING))
  {
    printf(" R");
  }
  else
  {
    printf(" Not-R");
  }
  if (this->IsBitSet(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS))
  {
    printf(" Not-Acc");
  }
  else
  {
    printf(" Acc");
  }
  if (this->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENTS))
  {
    printf(" Not-L");
  }
  else
  {
    printf(" L");
  }
  if (this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREAD_CLIENT))
  {
    printf(" M");
  }
  else
  {
    printf(" Not-M");
  }
  printf("\n");

  this->tcp_client_db_mgr->DisplayClientDb();
}
void TcpServerController::SetBit(uint32_t bit)
{
  // printf("SetBit called,state flag before | :%d, bit: %d\n", this->state_flags, bit);
  this->state_flags |= bit;
  // printf("SetBit called,state flag after | :%d, bit: %d\n", this->state_flags, bit);
}

bool TcpServerController::IsBitSet(uint32_t bit)
{
  /*
  if (bit == TCP_SERVER_NOT_LISTENING_CLIENTS)
  {
    printf("Now the sever flags: %d,bit is : %d \n", this->state_flags, bit);
    printf("after & :%d, %d\n", (this->state_flags & bit), bit);
  }*/
  return (this->state_flags & bit);
}

void TcpServerController::UnSetBit(uint32_t bit)
{
  this->state_flags &= ~bit;
}
void TcpServerController::StartConectionAcceptionSvc()
{
  if (this->IsBitSet(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS))
  {
    return;
  }
  this->UnSetBit(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
  this->tcp_new_conn_acc = new TcpNewConnectionAcceptor(this);
  this->tcp_new_conn_acc->StopTcpNewConnectionAcceptorThread();
}
void TcpServerController::StopConnectionAcceptingSvc()
{
  if (this->IsBitSet(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS))
  {
    return;
  }
  this->SetBit(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
  this->tcp_new_conn_acc = NULL;
}

void TcpServerController::StopClientSvcMgr()
{
  if (this->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENTS))
  {
    return;
  }
  this->tcp_client_svc_mgr->Stop();
  this->SetBit(TCP_SERVER_NOT_LISTENING_CLIENTS);
  this->tcp_client_svc_mgr = NULL;
}
void TcpServerController::StartClientSvcMgr() {}
void TcpServerController::CopyAllClientsTolist(std::list<TcpClient *> *list) {}

void TcpServerController::Stop()
{
  TcpClient *tcp_client;
  if (this->tcp_new_conn_acc)
  {
    this->StopConnectionAcceptingSvc();
    this->SetBit(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
  }
  if (this->tcp_client_svc_mgr)
  {
    this->StopClientSvcMgr();
    this->SetBit(TCP_SERVER_NOT_LISTENING_CLIENTS);
  }
  /*Stop the abolve two services first ensures that, no thread is alive which could add tcp client
  back inot data base*/
  this->tcp_client_db_mgr->Purge();
  delete this->tcp_client_db_mgr;
  this->tcp_client_db_mgr = NULL;

  this->UnSetBit(TCP_SERVER_RUNNING);
  delete this;
}

void TcpServerController::CreateActiveAClient(uint32_t server_ip_addr, uint16_t server_port_no)
{
  // Create an active client by itsself
  TcpClient *tcp_client = new TcpClient(this->ip_addr, this->port_no);
  tcp_client->SetState(TCP_CLIENT_STATE_ACTIVE_OPENER);
  tcp_client->SetState(TCP_CLIENT_STATE_CONNECTED);
  tcp_client->server_ip_addr = server_ip_addr;
  tcp_client->server_port_no = server_port_no;
  // connect to another server
  int sockfd = 0;
  struct sockaddr_in dest;
  dest.sin_family = AF_INET;
  dest.sin_port = htons(server_port_no);
  char ip_addr[16];
  struct hostent *host = (struct hostent *)gethostbyname(network_convert_ip_n_to_p(server_ip_addr, ip_addr));
  dest.sin_addr = *((struct in_addr *)host->h_addr);
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  int rc = connect(sockfd, (struct sockaddr *)&dest, sizeof(sockaddr));
  if (!rc)
  {
    printf("connected\n");
  }
  else
  {
    printf("connection failed!\n");
  }

  this->tcp_client_db_mgr->AddClienttoDb(tcp_client);
}