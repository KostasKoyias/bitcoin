CC = gcc
CFLAGS = -g -Wall 
HDRS = hashTable.h wallet.h tree.h coin.h list.h utils.h
OBJS = wallet.o tree.o hashTable.o list.o coin.o trans.o utils.o 
SOURCES = bitcoin.c wallet.c tree.c hashTable.c coin.c list.c trans.c utils.c

all: bitcoin $(OBJS)  

# main programm
bitcoin: bitcoin.c $(OBJS)
	$(CC) $(CFLAGS) -o bitcoin $(SOURCES)

# data structures and utilities	
coin.o: coin.c
	$(CC) $(CFLAGS) -c coin.c
	
wallet.o: wallet.c
	$(CC) $(CFLAGS) -c wallet.c

hashTable.o: hashTable.c
	$(CC) $(CFLAGS) -c hashTable.c

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

trans.o: trans.c
	$(CC) $(CFLAGS) -c trans.c

list.o: list.c
	$(CC) $(CFLAGS) -c list.c

tree.o: tree.c
	$(CC) $(CFLAGS) -c tree.c

clean: 
	rm -f $(OBJS) 
