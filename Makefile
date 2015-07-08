clean:
	rm -f server
fork:
	gcc -o fork fork.c

server: clean
	gcc -o server server.c

server2:
	gcc -o server2 server2.c
