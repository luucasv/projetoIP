#ifndef DEFAULT_H
#define DEFAULT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 9005
#define TOO_MANY_CLIENTS 1
#define SUCCESSFUL_CONNECTION 2
#define SELECT_TIMEOUT 300

#endif
