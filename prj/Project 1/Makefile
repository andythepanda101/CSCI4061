CFLAGS = -Wall -Werror -g
CC     = gcc $(CFLAGS) 



##### compilation #####
commando : commando.o cmd.o cmdcol.o util.o
	$(CC) -o $@ $^

commando.o : commando.c
	$(CC) -c $<

cmd.o : cmd.c
	$(CC) -c $<

cmdcol.o : cmdcol.c
	$(CC) -c $<

util.o : util.c
	$(CC) -c $<

clean :
	rm -f commando *.o


include test_Makefile
