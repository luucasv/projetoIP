#ifndef LIB_DEFAULT_H_
#define LIB_DEFAULT_H_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 9005

enum conn_msg_t {
  SUCCESSFUL_CONNECTION,
  TOO_MANY_CLIENTS,
  CONNECTIONS_CLOSED
};

#define SELECT_TIMEOUT 300

#endif  // LIB_DEFAULT_H_
