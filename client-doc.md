# Documentação client.h

Uma breve introdução sobre as funções implementadas na biblioteca **client.h** e em que situações podemos usar cada uma delas.

## Conectando ao servidor
```c
enum conn_ret_t connectToServer(const char *server_IP);
```
Uma das primeiras coisas que um cliente precisa fazer é se conectar ao servidor.  
Com esta função nos conectamos ao servidor com IP dado por server_IP. Para os que sabem um pouco mais sobre comunicação pode ser estranho não ser necessário informar a porta que servidor está escutando, mas este conceito foi abstraído e a porta é uma constante definida com mesmo valor para os dois códigos. 

##### **Parâmetros:**
-	**server_IP**:  Endereço para uma string (C like) contendo o endereço IP do servidor no formato ("A.B.C.D"). Quando o **server\_IP** é *NULL* o endereço buscado é o localhost (servidor rodando na mesma máquina).
##### **Retorno:**
-	Um elemento de enum *conn\_ret\_t*, representando o estado da connexão:
	- *SERVER_UP*, se conexão foi estabelecida
	- *SERVER_DOWN*, se não foi possivel encontrar o servidor
	- *SERVER_FULL*, se o server está cheio (atingiu o número máximo de clientes)
    - *SERVER_CLOSED*, se o server está fechado para conexões
    - *SERVER_TIMEOUT*, se o server demorou para dar uma resposta sobre o status da conexão
##### **OBS:** 
- O único retorno em que o programa deve seguir o fluxo normalmente é SERVER_UP.

## Enviando mensagens para o servidor
```c
int sendMsgToServer(void *msg, int size);
```
Envia a mensagem que está no endereço dado em **msg** ao servidor.
##### **Parâmetros:**
-	**msg**: Ponteiro para a mensagem que será enviada para o servidor.
-	**size**:   Tamanho da mensagem em bytes

##### **Retorno:**
- Caso o servidor tenha desconectado, o retorno é *SERVER_DISCONNECTED*
- Caso contrário,	um inteiro indicando a quantidade de bytes enviados, caso tudo ocorra bem esta quantidade é igual ao tamanho da mensagem.

## Recebendo mensagens do servidor
```c
int recvMsgFromServer(void *msg, int option);
```
Busca por uma mensagem que ainda não foi lida e foi enviada do servidor para o cliente. A mensagem vai ficar no endereço dado como argumento (**msg**). 
##### **OBS:** 

- Esta função pode ou ser não bloqueante, o que quer dizer que se não houver mensagem pendente a função pode(ou não) esperar para que uma mensagem seja recebida(esta funcionalidade é controlada através do parâmetro **option**).
##### **Parâmetros:**
-	**msg**: Um ponteiro para o local na memória onde desejamos salvar a mensagem que vai ser recebida.
-	**option**:   Deve ser uma das duas contantes:
	- *DONT\_WAIT* ou *WAIT\_FOR\_IT*
	- Como o nome sugere, quando utilizamos a primeira estamos indicando que não queremos esperar a mensagem ser recebida, já na segunda o programa só retorna quando uma mensagem for recebida.

##### **Retorno:**
- Caso o servidor tenha desconectado, o retorno é *SERVER_DISCONNECTED*
-	Caso o valor de **option** seja *DONT_WAIT* e não exista mensagens pendentes, o retorno é *NO_MESSAGE*.
-	Caso contrário, o retorno é a quantidade de bytes lidos.

## Funções de controle do cliente
```c
char getch();
```
Espera um curto período de tempo para que uma tecla seja pressionada. Caso nenhuma tecla seja pressionada retorna uma *flag* indicando que nenhuma foi pressionada.
Função muito útil quando não podemos utilizar as funções padrão para ler entrada e saída do teclado, quando se espera que toda a informação seja digitada antes de retornar(o que pode prejudicar o efeito de *real time* do jogo).

##### **Retorno:**
-	Caso alguma tecla tenha sido pressionada, retorna o character que foi pressionado.
-	Caso contrário, retorna a constante *NO_KEY_PRESSED*