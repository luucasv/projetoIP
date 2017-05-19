#ifndef CLIENT_H
#define CLIENT_H

#define NO_KEY_PRESSED '\0'

#define NO_MESSAGE -1
#define WAIT_FOR_IT 1
#define DONT_WAIT 2

void connectToServer(const char *server_IP);
int sendMsgToServer(void *msg, int size);
int recvMsgFromServer(void *msg, int option);
char getch();

#endif
