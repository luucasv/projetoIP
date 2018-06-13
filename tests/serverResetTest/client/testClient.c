#include "client.h"
#include <stdio.h>
#include <unistd.h>

int main() {
  for (int i = 0; i < 10; i++) {
    enum conn_ret_t ans = connectToServer(NULL);
    if (ans == SERVER_UP) puts("SERVER_UP");
    else if (ans == SERVER_DOWN) puts("SERVER_DOWN");
    else if (ans == SERVER_FULL) puts("SERVER_FULL");
    else if (ans == SERVER_CLOSED) puts("SERVER_CLOSED");
    else if (ans == SERVER_TIMEOUT) puts("SERVER_TIMEOUT");
    sleep(1);
    int var;
    while (recvMsgFromServer(&var, WAIT_FOR_IT) != SERVER_DISCONNECTED);
  }
}