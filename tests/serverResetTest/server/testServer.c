#include "server.h"
#include <stdio.h>
#include <unistd.h>

const int MAX_CLIENTS = 2;

int main() {
  serverInit(MAX_CLIENTS);
  int quant_clients = 0;
  while (1) {
    int id = acceptConnection();
    if (id != NO_CONNECTION) {
      quant_clients++;
    }
    if (quant_clients == MAX_CLIENTS) {
      sleep(1);
      serverReset();
      quant_clients = 0;
    }
  }
}