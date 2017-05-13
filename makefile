serverFile=examples/simpleChatServer
clientFile=examples/simpleChatClient
serverName=server
clientName=client

all: compServer compClient

server: compServer runServer

client: compClient runClient

compClient:
	gcc -o $(clientName) $(clientFile).c lib/client.c

compServer:
	gcc -o $(serverName) $(serverFile).c lib/server.c

runClient:
	./$(clientName)

runServer:
	./$(serverName)