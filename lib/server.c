#include "server.h"
#include "default.h"

struct server_view_client {
  int sockid;
};

typedef struct server_view_client server_view_client;

int actual_max_clients;  // user chosen max_clients
int clients_connected;   // how many clients are connected at moment
server_view_client *connected_clients = NULL;  // saves sockid for each client

fd_set server_fd_set, active_fd_set;
int server_sock;  // server socket id

struct msg_ret_t make_msg_ret(int status, int client_id, int size) {
  struct msg_ret_t ret = {status, client_id, size};
  return ret;
}

// Make socket for the server
int makeSocket() {
  int sock;
  struct sockaddr_in name;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Could not create socket");
    exit(EXIT_FAILURE);
  }
  int aux = 1;
  int set_ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &aux, sizeof(int));
  if (set_ret < 0) {
    perror("Could not set 'SO_REUSEADDR' option");
    exit(EXIT_FAILURE);
  }
  // assign localhost:PORT to the socket
  // where to find it
  name.sin_family = AF_INET;
  name.sin_port = htons(PORT);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  int bind_ret = bind(sock, (struct sockaddr *)&name, sizeof(name));
  if (bind_ret < 0) {
    perror("Server could not bind!");
    exit(EXIT_FAILURE);
  }
  return sock;
}

// Set everything nedded to the server
void serverInit(int max_clients) {
  if (max_clients <= 0) {
    perror("max_clients is invalid!");
    exit(EXIT_FAILURE);
  }

  actual_max_clients = max_clients;
  size_t bytes_size = (size_t)max_clients * sizeof(server_view_client);
  connected_clients =
      (server_view_client *)realloc(connected_clients, bytes_size);

  server_sock = makeSocket();

  // make it available for connections
  // second argument is maximum queue legth
  int listen_ret = listen(server_sock, 100);
  if (listen_ret < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  serverReset();
}

// Clean server
void serverReset() {
  int i;
  for (i = 0; i < actual_max_clients; ++i) {
    if (isValidId(i)) {
      disconnectClient(i);
    }
  }
  memset(connected_clients, 0,
         (size_t)actual_max_clients * sizeof(server_view_client));
  clients_connected = 0;
  FD_ZERO(&active_fd_set);
  FD_ZERO(&server_fd_set);
  FD_SET(server_sock, &server_fd_set);
}

void rejectConnection() {
  struct timeval timeout = {0, SELECT_TIMEOUT};
  fd_set readfds = server_fd_set;

  int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
  if (sel_ret < 0) {
    perror("select");
    exit(EXIT_FAILURE);
  }
  if (sel_ret == 0) {
    return;
  }

  int new_sock = accept(server_sock, NULL, NULL);
  enum conn_msg_t return_msg = CONNECTIONS_CLOSED;
  int msg_size = sizeof(return_msg);

  if (new_sock < 0) {  // not valid sock value
    return;
  }
  write(new_sock, &msg_size, sizeof(msg_size));
  write(new_sock, &return_msg, sizeof(return_msg));
  close(new_sock);
}

int acceptConnection() {
  struct timeval timeout = {0, SELECT_TIMEOUT};
  fd_set readfds = server_fd_set;

  int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
  if (sel_ret < 0) {
    perror("select");
    exit(EXIT_FAILURE);
  }
  if (sel_ret == 0) {
    return NO_CONNECTION;
  }

  int new_sock = accept(server_sock, NULL, NULL);
  enum conn_msg_t return_msg;

  if (new_sock < 0) {  // not valid sock value
    perror("Could not create new socket for new connection");
    exit(EXIT_FAILURE);
  }
  if (clients_connected == actual_max_clients) { 
    // there are too many clients connected
    return_msg = TOO_MANY_CLIENTS;
    int x = sizeof(return_msg);
    write(new_sock, &x, sizeof(x));
    write(new_sock, &return_msg, sizeof(return_msg));
    close(new_sock);
  } else {
    ++clients_connected;
    int i;
    return_msg = SUCCESSFUL_CONNECTION;
    // look for available space for this new client
    for (i = 0; i < actual_max_clients; ++i) { 
      if (connected_clients[i].sockid == 0) {
        int x = sizeof(return_msg);
        write(new_sock, &x, sizeof(x));
        write(new_sock, &return_msg, sizeof(return_msg));
        FD_SET(new_sock, &active_fd_set);
        connected_clients[i].sockid = new_sock;
        return i;
      }
    }
  }
  return NO_CONNECTION;
}

/*
Receive a pending message from cleints
Return:
  If there is no message, return.status is NO_MESSAGE
  If the user sent a message to disconect, return.status is DISCONNECT_MSG and
    return.client_id is the user's id (the user is automatically disconected )
  otherwise return.status is MESSAGE_OK , return.client_id  is the user's
    id and return.size is the size of the message in bytes
*/

struct msg_ret_t recvMsg(void *msg) {
  struct timeval timeout = {0, SELECT_TIMEOUT};
  fd_set readfds = active_fd_set;
  int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
  if (sel_ret < 0) {
    perror("select");
    exit(EXIT_FAILURE);
  }
  if (sel_ret == 0) {  // timeout
    return make_msg_ret(NO_MESSAGE, -1, 0);
  }
  int i;
  // look for someone with valid sockid
  for (i = 0; i < actual_max_clients; ++i) {
    if (isValidId(i)) {
      if (FD_ISSET(connected_clients[i].sockid, &readfds)) {
        return recvMsgFromClient(msg, i, WAIT_FOR_IT);
      }
    }
  }
  return make_msg_ret(NO_MESSAGE, -1, 0);
}

/*
Receive a pending message from cleint with id equals to client_id
Return:
  If client_id is not a valid one, return.status is NOT_VALID_CLIENT_ID
  If there is no message in buffer and option is DONT_WAIT, return.status is
NO_MESSAGE
  If there is no message in buffer and option is WAIT_FOR_IT,
  the function blocks(waits) untill the user sends a message
  When a message is received:
  If the user sent a message to disconect, return.status is DISCONNECT_MSG
  otherwise return.status is MESSAGE_OK  and return.size is the size of the
message in bytes
*/
struct msg_ret_t recvMsgFromClient(void *msg, int client_id, int option) {
  if (!isValidId(client_id)) {
    return make_msg_ret(NOT_VALID_CLIENT_ID, -1, 0);
  }
  if (option == DONT_WAIT) {
    struct timeval timeout = {0, SELECT_TIMEOUT};
    fd_set readfds = active_fd_set;
    int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
    if (sel_ret < 0) {
      perror("select");
      exit(EXIT_FAILURE);
    }
    if (sel_ret == 0 ||
        !FD_ISSET(connected_clients[client_id].sockid, &readfds)) {  // timeout
      return make_msg_ret(NO_MESSAGE, -1, 0);
    }
  }
  // either we have to wait, or there is nothing to wait
  int msg_size;
  // get message size
  ssize_t size_ret = read(connected_clients[client_id].sockid, &msg_size, 
                          sizeof(int));
  if (size_ret <= 0) {
    disconnectClient(client_id);
    return make_msg_ret(DISCONNECT_MSG, client_id, 0);
  }
  // get message content
  ssize_t total_size = 0;
  while (total_size < msg_size) {
    ssize_t msg_ret = read(connected_clients[client_id].sockid,
                           msg + total_size, (size_t)msg_size);
    if (msg_ret <= 0) {
      disconnectClient(client_id);
      return make_msg_ret(DISCONNECT_MSG, client_id, 0);
    }
    total_size += msg_ret;
  }
  return make_msg_ret(MESSAGE_OK, client_id, (int)total_size);
}

/*
Send message msg with size being its length in bytes to client with client_id
Return:
  If client_id is not a valid one, returns NOT_VALID_CLIENT_ID
  Otherwise returns the length of the message sent
*/
int sendMsgToClient(void *msg, int size, int client_id) {
  if (!isValidId(client_id)) {
    return NOT_VALID_CLIENT_ID;
  }
  ssize_t size_ret = send(connected_clients[client_id].sockid, &size,
                          sizeof(int), MSG_NOSIGNAL);
  if (size_ret <= 0) {
    return CLIENT_DISCONNECTED;
  }
  ssize_t total_size = 0;
  while (total_size < (ssize_t) size) {
    ssize_t msg_ret = send(connected_clients[client_id].sockid,
                           msg + total_size, (size_t)size, MSG_NOSIGNAL);
    if (msg_ret <= 0) {
      return CLIENT_DISCONNECTED;
    }
    total_size += msg_ret;
  }
  return (int)total_size;
}

/*
Send msg, with size being its length in bytes, to client every client connected
*/
void broadcast(void *msg, int size) {
  int i;
  for (i = 0; i < actual_max_clients; ++i) {
    if (isValidId(i)) {
      sendMsgToClient(msg, size, i);
    }
  }
}

/*
Force a client to disconnect from the server
*/
void disconnectClient(int client_id) {
  if (!isValidId(client_id)) {
    return;
  }
  close(connected_clients[client_id].sockid);
  FD_CLR(connected_clients[client_id].sockid, &active_fd_set);
  connected_clients[client_id].sockid = 0;
  --clients_connected;
}

/*
Check if client_id is a valid id
Return:
  false if it is not valid
  true  if it is valid
*/
int isValidId(int client_id) {
  return client_id < actual_max_clients &&
         connected_clients[client_id].sockid != 0;
}
