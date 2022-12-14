CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: echo

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

client.o: client.c csapp.h
	$(CC) $(CFLAGS) -c client.c

server.o: server.c csapp.h
	$(CC) $(CFLAGS) -c server.c

client: client.o csapp.o
	$(CC) $(CFLAGS) client.o csapp.o -o client $(LDFLAGS)

server: server.o csapp.o
	$(CC) $(CFLAGS) server.o csapp.o -o server $(LDFLAGS)
# Creates a tarball in ../proxylab-handin.tar that you can then
# hand in. DO NOT MODIFY THIS!
handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -f *~ *.o client server core *.tar *.zip *.gzip *.bzip *.gz
