CC = gcc
FLAGS = -Wall -g

.PHONY: all clean

all: sender reciever

sender: TCP_Sender.o
	$(CC) $(FLAGS) -o sender $^

reciever: TCP_Reciever.o
	$(CC) $(FLAGS) -o reciever $^

sender.o: TCP_Sender.c
	$(CC) $(FLAGS) -c sender.c -o sender

reciever.o: TCP_Reciever.c
	$(CC) $(FLAGS) -c TCP_Reciever.c -o TCP_Reciever.o


clean:
	rm -f sender reciever