#include "client.h"
#include <poll.h>
#include <termios.h>
#include "default.h"

#define GETCH_TIMEOUT 100
#define READ_CONN_TIMEOUT 6
int network_socket;
fd_set sock_fd_set;


void closeConnection() {
  shutdown(network_socket, 3);
  close(network_socket);
}

/*
  Connects to server with IP informed as string on standard format
(X.X.X.X)
  If ServerIP is NULL connects to localhost (good to debug)
*/
enum conn_ret_t connectToServer(const char *server_IP) {
  // create a socket for the client
  network_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (network_socket == -1) {
    perror("Could not create socket");
    exit(EXIT_FAILURE);
  }
  // making struct for server adress
  struct sockaddr_in server_address;
  memset(&server_address, 0, sizeof(server_address));  // clear struct
  server_address.sin_family = AF_INET;                 // set family
  server_address.sin_port = htons(PORT);               // working port
  if (server_IP == NULL) {  // if no IP sent, connect to localhost
    server_address.sin_addr.s_addr = INADDR_ANY;
  } else {
    server_address.sin_addr.s_addr = inet_addr(server_IP);
  }

  // Connect to server now
  int connection_status =
      connect(network_socket, (struct sockaddr *)&server_address,
              sizeof(server_address));
  enum conn_msg_t server_response = SUCCESSFUL_CONNECTION;

  FD_ZERO(&sock_fd_set);
  FD_SET(network_socket, &sock_fd_set);
  
  if (connection_status == 0) {
    // read server_response
    ssize_t conn_ans = recvMsgFromServer(&server_response, DONT_WAIT);
    unsigned int i = 0;
    while (i < READ_CONN_TIMEOUT && conn_ans == NO_MESSAGE) {
      i++;
      sleep(i);
      conn_ans = recvMsgFromServer(&server_response, DONT_WAIT);
    }
    if (conn_ans == SERVER_DISCONNECTED) {
      closeConnection();
      return SERVER_DOWN;
    } else if (conn_ans == NO_MESSAGE) {
      closeConnection();
      return SERVER_TIMEOUT;
    }
  } else if (connection_status == -1) {
    closeConnection();
    return SERVER_DOWN;
  }

  if (server_response == TOO_MANY_CLIENTS) {
    closeConnection();
    return SERVER_FULL;
  } else if (server_response == CONNECTIONS_CLOSED) {
    closeConnection();
    return SERVER_CLOSED;
  }

  FD_ZERO(&sock_fd_set);
  FD_SET(network_socket, &sock_fd_set);
  return SERVER_UP;
}

int sendMsgToServer(void *msg, int size) {
  ssize_t size_ret = send(network_socket, &size, sizeof(int), MSG_NOSIGNAL);
  if (size_ret <= 0) {
    return SERVER_DISCONNECTED;
  }
  ssize_t total_size = 0;
  while (total_size < (ssize_t) size) {
    ssize_t msg_ret =
        send(network_socket, msg + total_size, (size_t)size, MSG_NOSIGNAL);
    if (msg_ret <= 0) {
      return SERVER_DISCONNECTED;
    }
    total_size += msg_ret;
  }
  return (int)total_size;
}

int recvMsgFromServer(void *msg, int option) {
  if (option == DONT_WAIT) {
    struct timeval timeout = {0, SELECT_TIMEOUT};
    fd_set readfds = sock_fd_set;
    int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
    if (sel_ret < 0) {
      perror("select");
      exit(EXIT_FAILURE);
    }
    if (sel_ret == 0 || !FD_ISSET(network_socket, &readfds)) {  // timeout
      return NO_MESSAGE;
    }
  }
  int size;
  // get message size
  ssize_t size_ret = read(network_socket, &size, sizeof(int));
  if (size_ret <= 0) {
    return SERVER_DISCONNECTED;
  }
  // get message content
  ssize_t total_size = 0;
  while (total_size < (ssize_t) size) {
    ssize_t msg_ret = read(network_socket, msg + total_size, (size_t)size);
    if (msg_ret <= 0) {
      return SERVER_DISCONNECTED;
    }
    total_size += msg_ret;
  }
  return (int)total_size ;
}

/*
    modified getch();
    waits GETCH_TIMEOUT ms for key to be pressed, if it is not, returns
   NO_KEY_PRESSED
*/

char getch() {
  struct termios oldt, newt;
  int ch;
  struct pollfd mypoll = {STDIN_FILENO, POLLIN | POLLPRI, 0};
  tcgetattr(STDIN_FILENO, &oldt);  // saving old config in oldt

  newt = oldt;
  newt.c_lflag &= ~((tcflag_t)(ICANON | ECHO));

  tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // setting new config

  if (poll(&mypoll, 1, GETCH_TIMEOUT)) {  // if key is pressed before timeout
    ch = getchar();
  } else {
    ch = NO_KEY_PRESSED;
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return (char)ch;
}
