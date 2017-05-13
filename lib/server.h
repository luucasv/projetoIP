#ifndef SERVER_H
#define SERVER_H

#define MAX_CLIENTS 100

#define NO_CONNECTION -1
#define NO_MESSAGE -2
#define NOT_VALID_CLIENT_ID -3

#define WAIT_FOR_IT 1
#define DONT_WAIT 2

void serverInit(int max_clients);
void serverReset();
int acceptConnection();
int recvMsg(void *msg);
int recvMsgFromClient(void *msg, int client_id, int option);
int sendMsgToClient(void *msg, int size, int client_id);
void broadcast(void *msg, int size);
void disconnectClient(int client_id);

#endif