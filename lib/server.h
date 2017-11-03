#ifndef SERVER_H
#define SERVER_H

#define MESSAGE_OK 0
#define NO_CONNECTION -1
#define NO_MESSAGE -2
#define NOT_VALID_CLIENT_ID -3
#define DISCONNECT_MSG -4

#define WAIT_FOR_IT 1
#define DONT_WAIT 2

struct msg_ret_t {
  int status, client_id, size;
};

void serverInit(int max_clients);
void serverReset();
int acceptConnection();
struct msg_ret_t recvMsg(void *msg);
struct msg_ret_t recvMsgFromClient(void *msg, int client_id, int option);
int sendMsgToClient(void *msg, int size, int client_id);
void broadcast(void *msg, int size);
void disconnectClient(int client_id);
int isValidId(int client_id);

#endif
