# Documentação server.h

Uma breve introdução sobre as funções implementadas na biblioteca **server.h** e em que situações podemos usar cada uma delas.

## Funções iniciais o server
### Inicializando o servidor
```c
void serverInit(int max_clients);
``` 
Inicializa variáveis internas importantes para o servidor, esta função dever ser chamada sempre uma única vez no código de qualquer server\.

##### **Parâmetros:** 

- **max\_clients**: Um número entre 1 e *MAX_CLIENTS* (definido na *server.h*)\.

### Resetando o servidor
```c
void serverReset();
``` 
Limpa todas as informações salvas no servidor atualmente. Esta função deixa o servidor com estado igual ao que ele tinha logo após ser inicializado(*serverInit*)\.

## Aceitando conexões

```c
int acceptConnection();
``` 
Busca por uma conexão nova, caso tenha alguma pendente.  

##### **Retorno:**
-  Caso haja alguma conexão pendente a função retorna o id que foi dado para o cliente. **O id é um número entre [0, *max_clients*) (onde *max_clients* dado como argumento de serverInit**).  
-  Caso não exista conexões pendentes o retorno da função é *NO_CONNECTION* (definido em *server.h*)

```c
void rejectConnection();
``` 
Rejeita uma conexão nova, caso tenha alguma pendente.
É util para informar aos clientes que o servidor não está aberto a conexões.
Esta chamada causa um SERVER_CLOSED como resposta do client ao tentar conectar ao server.

### **OBS:** 

- É importante fazer uso destas funções, mesmo que para rejeitar uma conexão. Pois assim o cliente vai saber como anda o estado atual do servidor, e não receber simplesmente um TIMEOUT.

## Recebendo mensagens dos clientes
### Recebendo mensagens sem especificar o cliente
```c
struct msg_ret_t recvMsg(void *msg);
``` 
Busca por uma mensagem que ainda não foi lida e foi enviada por algum dos clientes conectados. A mensagem vai ficar no endereço dado como argumento.  

##### **OBS:** 

- Quando um cliente se desconecta do servidor, este manda automaticamente uma mensagem que sinaliza que está se desconectando.
- Esta é uma função não bloqueante, o que quer dizer que se não houver mensagem pendente a função não espera por uma e simplesmente retorna com uma flag indicando isto.  

##### **Parâmetros:**

- **msg**: Um ponteiro para o local na memória onde desejamos salvar a mensagem que vai ser recebida.  

##### **Retorno:**

- O retorno da função é do tipo struct msg_ret_t esta estrutura conta com os campos:
	- **client\_id**, **status** e **quant\_bytes**;
- Quando existem mensagens que não foram processadas ainda, o valor do campo **status** é igual a *MESSAGE\_OK*, em **cleint\_id** temos o id do cliente que enviou a mensagem e **quant\_bytes** o tamanho em bytes da mensagem que foi recebida.
- Caso contrário, se algum usuário enviou uma mensagem para informar que está se desconectando, no campo **status** temos o valor *DISCONNECT\_MSG* e no campo **client\_id** o id do cliente que enviou tal mensagem.
- Caso contrário, se não há mensagens não processadas, em **status** temos valor igual a *NO\_MESSAGE*

### Recebendo mensagens de um cliente específico
```c
struct msg_ret_t recvMsgFromClient(void *msg, int client_id, int option);
``` 
Busca por uma mensagem que ainda não foi lida e foi enviada pelo cliente com id igual a **client_id**. A mensagem vai ficar no endereço dado como argumento.

##### **OBS:** 

- Quando um client se desconecta do servidor, este manda automaticamente uma mensagem que sinaliza que está se desconectando.
- Esta função pode ou ser não bloqueante, o que quer dizer que se não houver mensagem pendente a função pode(ou não) esperar para que uma mensagem seja enviada(esta funcionalidade é controlada através dos parâmetros).

##### **Parâmetros**:

- **msg**: Um ponteiro para o local na memória onde desejamos salvar a mensagem que vai ser recebida.
- **client\_id**: Diferente da função anterior (*recvMsg*), esta função recebe as mensagens de um cliente específico, dado por **client\_id**.
- **option**: Deve ser uma das duas constantes:
	- *DONT\_WAIT* e *WAIT\_FOR\_IT*
	- Como o nome sugere, quando utilizamos a primeira estamos indicando que não queremos esperar a mensagem ser recebida, já na segunda o programa só retorna quando uma mensagem for recebida pelo servidor do cliente identificado pelo **client_id**

##### **Retorno:**

- O retorno da função é do tipo struct msg_ret_t esta estrutura conta com os campos:
	- **client\_id**, **status** e **quant\_bytes**;
- Quando existem mensagens que não foram processadas ainda (ou quando vamos esperar até que uma mensagem seja recebida e o cliente enviou uma "mensagem comum"), o valor do campo **status** é igual a *MESSAGE\_OK*, em **cleint\_id** temos o id do cliente que enviou a mensagem e **quant\_bytes** o tamanho em bytes da mensagem que foi recebida.
- Caso contrário, se algum usuário enviou uma mensagem para informar que está se desconectando, no campo **status** temos o valor *DISCONNECT\_MSG* e no campo **client_id** o id do cliente que enviou tal mensagem.
- Caso contrário, se não há mensagens não processadas, em **status** temos valor igual a *NO_MESSAGE*
- Por fim, se o id dado em **client\_id** não é válido no campo **status** temos valor igual ao da constante *NOT_VALID_CLIENT_ID*

## Enviando mensagens para os clientes
### Enviando mensagens para um único cliente
```c
int sendMsgToClient(void *msg, int size, int client_id);
``` 
Envia a mensagem que está no endereço dado em **msg** para o cliente com id igual a **client_id**

##### **Parâmetros:**

- **msg**: Um ponteiro para o endereço na memória onde vamos obter a mensagem que vamos enviar.
- **client\_id**: Id do cliente para o qual enviaremos a mensagem.
- **size**: O tamanho da mensagem em bytes.

##### **Retorno:**

- Um inteiro indicando a quantidade de bytes enviados, caso tudo ocorra bem está quantidade é igual ao tamanho da mensagem.

### Enviando mensagens para vários clientes de uma única vez
```c
void broadcast(void *msg, int size);
```
Envia a mensagem dada em **msg** para todos os clientes conectados no servidor.

##### **Parâmetros:**
- **msg**: Um ponteiro para o endereço na memória onde vamos obter a mensagem que vamos enviar.
- **size**: O tamanho da mensagem em bytes.

## Funções de controle do servidor
### Desconectando clientes
```c
void disconnectClient(int client_id);
```
Desconecta o cliente do servidor, dessa maneira o servidor não vai mais escutar mensagens que serão enviadas por este cliente, e este id estará liberado para que um novo cliente possa usá-lo.

##### **Parâmetros:**
- **client\_id**: O id do cliente que deseja desconectar.

### Verificando um ID
```c
int isValidId(int client_id);
```
Verifica se um dado **client\_id** é válido ou não.

##### **Parâmetros:**

- **client\_id**: O id que deseja verificar a validade.

##### **Retorno:** 
- O retorno é 0 se o **client_id** não é um id válido, e diferente de 0 se o id é válido.