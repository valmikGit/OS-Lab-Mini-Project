CC = gcc
# CFLAGS = -Wall -Wextra -O2 # LIsts all the possible warnings
LDFLAGS =

# Define targets for both the client and the server
all: client server

# Client binary
client: client.o
	$(CC) $(LDFLAGS) -o client client.o

# Server binary
server: server.o
	$(CC) $(LDFLAGS) -o server server.o

# Generic rule for building object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target for removing built files
clean:
	rm -f *.o client server

# Phony targets ensure make doesn't confuse them with files of the same name
.PHONY: all clean
