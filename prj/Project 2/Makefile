##### make file for chat thingy, project 2 #####

CFLAGS = -Wall -Werror -g
CC     = gcc $(CFLAGS)



##### compilation #####

##### compile server and client #####
bl_server: bl_server.o server_funcs.o util.o
	$(CC) -o $@ $^

bl_client: bl_client.o simpio.o
	$(CC) -o $@ $^ -lpthread

##### compile dependencies #####

bl_server.o : bl_server.c blather.h
	$(CC) -c $<

bl_client.o : bl_client.c blather.h
	$(CC) -c $<

server_funcs.o : server_funcs.c blather.h
	$(CC) -c $<

util.o : util.c
	$(CC) -c $<

simpio.o : simpio.c
	$(CC) -c $<

##### clean #####

clean :
	rm -f bl_server bl_client *.o *.fifo

clean-bl_server :
	rm -f bl_server bl_server.o

clean-bl_client :
	rm -f bl_client bl_client.o


include test_Makefile
