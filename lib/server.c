#include "server.h"
#include "default.h"


struct server_view_client{
	int sockid;
};

int actual_max_clients; // user chosen max_clients
int clients_connected; // how many clients are connected at moment
struct server_view_client connected_clients[MAX_CLIENTS]; // saves sockid for each client

fd_set server_fd_set, active_fd_set;
int server_sock; // server socket id



// Make socket for the server
int makeSocket(){
  	int sock;
  	struct sockaddr_in name;

  	sock = socket (AF_INET, SOCK_STREAM, 0);
  	if (sock < 0){
		puts("Could not create socket");
    	exit (EXIT_FAILURE);
  	}

  	// assign localhost:PORT to the socket
  	// where to find it
  	name.sin_family = AF_INET;
  	name.sin_port = htons (PORT);
  	name.sin_addr.s_addr = htonl (INADDR_ANY);
  	int bind_ret = bind (sock, (struct sockaddr *) &name, sizeof (name));
  	if(bind_ret < 0){
    	puts("Server could not bind!");
      	exit (EXIT_FAILURE);
    }

    // As the server doesn't uses threads, we need to make the "read" method not preemptive 
	// So it's making the read and the accept call not to stop the program and return "some defined message" when nothing is buffered

  	return sock;
}

// Set everything nedded for the server
void serverInit(int max_clients){
	if(max_clients <= 0 || max_clients >= MAX_CLIENTS){
		puts("max_clients is invalid!");
		exit(EXIT_FAILURE);
	}
	
	actual_max_clients = max_clients;
	server_sock = makeSocket();

	// make it available for connections
	// second argument is maximum queue legth
	int listen_ret = listen (server_sock, 100);
	if(listen_ret < 0){
	    perror("listen");
	    exit(EXIT_FAILURE);
	}

	serverReset();
}

void serverReset(){
	memset(connected_clients, 0, sizeof connected_clients);
	clients_connected = 0;
	FD_ZERO (&active_fd_set);
	FD_ZERO (&server_fd_set);
	FD_SET (server_sock, &server_fd_set);
}

/*
If there is a connection pedding, accept it.
Returns:
	NO_CONNECTION if there is no connection or failed (too many clients)
	otherwise returns a id to the new user (this id has to be used in other used when message is needed to be set to specific client)
*/
int acceptConnection(){
	struct timeval timeout = {0, SELECT_TIMEOUT};
	fd_set readfds = server_fd_set;

	int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
	if(sel_ret < 0){
        perror ("select");
        exit (EXIT_FAILURE);
   	}
   	if(sel_ret == 0){
   		return NO_CONNECTION;
   	}

	int new_sock = accept(server_sock, NULL, NULL);
	int return_msg;

	
	if(new_sock < 0){ // not valid sock value
		puts("Could not create new socket for new connection");
        exit(EXIT_FAILURE);
	}
	if(clients_connected == actual_max_clients){ // there are too many clients connected
		return_msg = TOO_MANY_CLIENTS;
		write(new_sock, &return_msg, sizeof(int));
		close(new_sock);
	}
	else{
		++clients_connected;
		int i;
		return_msg = SUCCESSFUL_CONNECTION;
		for(i = 0; i < actual_max_clients; ++i){ // look for available space for this new client
			if(connected_clients[i].sockid == 0){
				write(new_sock, &return_msg, sizeof(int));
				FD_SET(new_sock, &active_fd_set);
				connected_clients[i].sockid = new_sock;
				return i;
			}
		}
	}
   	return NO_CONNECTION;
}

/*
Receive a padding message from cleints
Return:
	If there is no message, returns NO_MESSAGE
	otherwise returns the ID of the client who sent the message
*/

int recvMsg(void * msg){
	struct timeval timeout = {0, SELECT_TIMEOUT};
	fd_set readfds = active_fd_set;
	int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
	if(sel_ret < 0){
        perror ("select");
        exit (EXIT_FAILURE);
   	}
   	if(sel_ret == 0){ // timeout
		return NO_MESSAGE;
	}
	int i;
	for(i = 0; i < actual_max_clients; ++i){ // look for someone with valid sockid
		if(connected_clients[i].sockid != 0){
			if(FD_ISSET(connected_clients[i].sockid, &readfds)){
				recvMsgFromClient(msg, i, WAIT_FOR_IT);
				return i;
			}
		}
	}
	return NO_MESSAGE;
}

/*
Receive a padding message from cleint with id equals to client_id
Return:
	If client_id is not a valid one, returns NOT_VALID_CLIENT_ID
	If there is no message, returns NO_MESSAGE
	otherwise returns the received message size in bytes
*/
int recvMsgFromClient(void *msg, int client_id, int option){
	if(connected_clients[client_id].sockid == 0){
		return NOT_VALID_CLIENT_ID;
	}
	if(option == DONT_WAIT){
		struct timeval timeout = {0, SELECT_TIMEOUT};
		fd_set readfds = active_fd_set;
		int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
		if(sel_ret < 0){
	        perror ("select");
	        exit (EXIT_FAILURE);
	   	}
	   	if(sel_ret == 0 || !FD_ISSET(connected_clients[client_id].sockid, &readfds)){ // timeout
			return NO_MESSAGE;
		}
	}
	// either we have to wait, or there is nothing to wait
	int size;
	int size_ret, msg_ret;
	// get message size
	size_ret = read(connected_clients[client_id].sockid, &size, sizeof(int));
	if(size_ret < 0){
		disconnectClient(client_id);
		return NO_MESSAGE;
	}
	// get message content
	msg_ret = read(connected_clients[client_id].sockid, msg, size);
	if(msg_ret < 0){
		disconnectClient(client_id);
		return NO_MESSAGE;
	}
	return msg_ret;
}

/*
Send message to client with client_id, size is size of message in bytes
Return:
	If client_id is not a valid one, returns NOT_VALID_CLIENT_ID
	If successfull size in bytes of message sent(read about write function)
	If error, stop program
*/
int sendMsgToClient(void *msg, int size, int client_id){
	if(connected_clients[client_id].sockid == 0){
		return NOT_VALID_CLIENT_ID;
	}
	int size_ret = write(connected_clients[client_id].sockid, &size, sizeof(int));
	if(size_ret < 0){
		puts("Failed to send message to client");
		exit(EXIT_FAILURE);
	}
	int msg_ret = write(connected_clients[client_id].sockid, msg, size);
	if(msg_ret < 0){
		puts("Failed to send message");
		exit(EXIT_FAILURE);
	}
	return msg_ret;
}

/*
Send message to client each client connected, size is size of message in bytes
Return:
	Internaly calls sendMsgToClient, so if on error the program will stop
*/
void broadcast(void *msg, int size){
	int i;
	for(i = 0; i < actual_max_clients; ++i){
		if(connected_clients[i].sockid != 0){
			sendMsgToClient(msg, size, i);
		}
	}
}

void disconnectClient(int client_id){
	close(connected_clients[client_id].sockid);
	FD_CLR (connected_clients[client_id].sockid, &active_fd_set);
	connected_clients[client_id].sockid = 0;
	--clients_connected;
}