#ifndef LIB_CLIENT_H_
#define LIB_CLIENT_H_

#define NO_KEY_PRESSED '\0'

#define NO_MESSAGE -1
#define SERVER_DISCONNECTED -2
#define WAIT_FOR_IT 1
#define DONT_WAIT 2

enum conn_ret_t {
  SERVER_UP,
  SERVER_DOWN,
  SERVER_FULL,
  SERVER_CLOSED,
  SERVER_TIMEOUT
};

void closeConnection();
enum conn_ret_t connectToServer(const char *server_IP);
int sendMsgToServer(void *msg, int size);
int recvMsgFromServer(void *msg, int option);
char getch();

#endif  // LIB_CLIENT_H_
