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

enum connect_answer {
	TOO_MANY_CLIENTS,
	SUCCESSFUL_CONNECTION
} connect_answer_t;

#define SELECT_TIMEOUT 300

#endif
