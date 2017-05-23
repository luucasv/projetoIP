#include "client.h"
#include "default.h"
#include <termios.h>
#include <poll.h>

#define GETCH_TIMEOUT 100
int network_socket;
fd_set sock_fd_set;

/*
	Connects to server with IP informed as string on standard format (X.X.X.X)
	If ServerIP is NULL connects to localhost (good to debug)
*/
void connectToServer(const char *server_IP){
	// create a socket for the client
	network_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(network_socket == -1){
		perror("Could not create socket");
		exit(EXIT_FAILURE);
	}

	// making struct for server adress
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address)); // clear struct
	server_address.sin_family = AF_INET; // set family
	server_address.sin_port = htons(PORT); // working port
	if(server_IP == NULL){ // if no IP sent, connect to localhost
		server_address.sin_addr.s_addr = INADDR_ANY;
	}
	else{
		server_address.sin_addr.s_addr = inet_addr(server_IP);
	}

	// Connect to server now
	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	int server_response = SUCCESSFUL_CONNECTION;
	if(connection_status == 0){
		read(network_socket, &server_response, sizeof(int)); // reads server_response
	}
	else if(connection_status == -1){
		printf("ERROR!! Connection was not succefull\n");
		exit(EXIT_FAILURE);
	}
	if(server_response == TOO_MANY_CLIENTS){
		perror("Too many clients connected!");
		exit(EXIT_FAILURE);
	}

	FD_ZERO (&sock_fd_set);
	FD_SET (network_socket, &sock_fd_set);
}


int sendMsgToServer(void *msg, int size){
	int size_ret = write(network_socket, &size, sizeof(int));
	if(size_ret < 0){
		perror("Failed to send message");
		exit(EXIT_FAILURE);
	}
	int msg_ret = write(network_socket, msg, size);
	if(msg_ret < 0){
		perror("Failed to send message");
		exit(EXIT_FAILURE);
	}
	return msg_ret;
}

int recvMsgFromServer(void* msg, int option){
	if(option == DONT_WAIT){
		struct timeval timeout = {0, SELECT_TIMEOUT};
		fd_set readfds = sock_fd_set;
		int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
		if(sel_ret < 0){
	        perror ("select");
	        exit (EXIT_FAILURE);
	   	}
	   	if(sel_ret == 0 || !FD_ISSET(network_socket, &readfds)){ // timeout
			return NO_MESSAGE;
		}
	}
	// either we have to wait, or there is nothing to wait
	int size;
	int size_ret, msg_ret;
	// get message size
	size_ret = read(network_socket, &size, sizeof(int));
	// get message content
	msg_ret = read(network_socket, msg, size);
	return msg_ret;
}

/*
	modified getch();
	waits GETCH_TIMEOUT ms for key to be pressed, if it is not, returns NO_KEY_PRESSED
*/

char getch(){
	struct termios oldt, newt;
    int ch;
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
    tcgetattr(STDIN_FILENO, &oldt); // saving old config in oldt

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // setting new config

    if ( poll(&mypoll, 1, GETCH_TIMEOUT)){ // if key is pressed before timeout
	    ch = getchar();
	}
	else{
		ch = NO_KEY_PRESSED;
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
