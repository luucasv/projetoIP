#include "../lib/client.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MSG_MAX_SIZE 350
#define BUFFER_SIZE (MSG_MAX_SIZE + 100)
#define LOGIN_MAX_SIZE 13
#define HIST_MAX_SIZE 200

int main(){
	connectToServer(NULL);
	char str_buffer[BUFFER_SIZE], type_buffer[MSG_MAX_SIZE] = {0};
	char msg_history[HIST_MAX_SIZE][MSG_MAX_SIZE] = {{0}};
	int type_pointer = 0;

	printf("Digite o seu login desejado (limite = %d): ", LOGIN_MAX_SIZE);
	fgets(str_buffer, LOGIN_MAX_SIZE, stdin);
	int len = strlen(str_buffer) - 1;
	str_buffer[len] = '\0'; // chaging \n por \0
	sendMsgToServer((void *) str_buffer, len + 1); // size includes \0


	puts("Welcome to the chat example");
	puts("Just type your messages e talk to your freinds");
	while(1){
		//// LER UMA TECLA DIGITADA
		char ch = getch();
		if(ch == '\n'){
			type_buffer[type_pointer++] = '\0';
			sendMsgToServer((void *) type_buffer, type_pointer);
			type_pointer = 0;
			type_buffer[type_pointer] = '\0';
		}
		else if(ch == 127 || ch == 8){
			if(type_pointer > 0){
				--type_pointer;
				type_buffer[type_pointer] = '\0';
			}
		}
		else if(ch != NO_KEY_PRESSED && type_pointer+1 < MSG_MAX_SIZE){
			type_buffer[type_pointer++] = ch;
			type_buffer[type_pointer] = '\0';
		}

		//// LER UMA MENSAGEM DO SERVIDOR
		int ret = recvMsgFromServer(str_buffer, DONT_WAIT);
		if(ret != NO_MESSAGE){
			int i;
			for(i = 0; i < HIST_MAX_SIZE - 1; ++i){
				strcpy(msg_history[i], msg_history[i+1]);
			}
			strcpy(msg_history[HIST_MAX_SIZE - 1], str_buffer);
		}

		//// PRINT NEW CHAT STATE
		system("clear");
		int i;
		for(i = 0; i < HIST_MAX_SIZE; ++i){
			printf("%s\n", msg_history[i]);
		}
		printf("\nSua mensagem: %s\n", type_buffer);

	}
	return 0;
}