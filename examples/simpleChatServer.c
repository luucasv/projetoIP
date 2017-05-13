#include "../lib/server.h"
#include <stdio.h>
#include <string.h>

#define MSG_MAX_SIZE 350
#define BUFFER_SIZE (MSG_MAX_SIZE + 100)
#define LOGIN_MAX_SIZE 13
#define MAX_CHAT_CLIENTS 3

int main(){
	char client_names[MAX_CHAT_CLIENTS][LOGIN_MAX_SIZE];
	char str_buffer[BUFFER_SIZE], aux_buffer[BUFFER_SIZE];
	serverInit(MAX_CHAT_CLIENTS);
	while(1){
		int id = acceptConnection();
		if(id != NO_CONNECTION){
			recvMsgFromClient(client_names[id], id, WAIT_FOR_IT);
			strcpy(str_buffer, client_names[id]);
			strcat(str_buffer, " connected to chat");
			broadcast(str_buffer, strlen(str_buffer) + 1);
			printf("%s connected id = %d\n", client_names[id], id);
		}

		id = recvMsg(aux_buffer);
		if(id != NO_MESSAGE){
			sprintf(str_buffer, "%s-%d: %s", client_names[id], id, aux_buffer);
			broadcast(str_buffer, strlen(str_buffer) + 1);
		}
	}
}