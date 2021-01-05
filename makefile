All: server client
server: makeserver.c
	gcc makeserver.c -o server
client: makeclient.c
	gcc makeclient.c -o client
