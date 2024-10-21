# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Executable names
CLIENT = client
SERVER = server

# Build the client and server programs
all: $(CLIENT) $(SERVER) clean_o

# Compile client.c
$(CLIENT): client.o
	$(CC) $(CFLAGS) -o $(CLIENT) client.o

# Compile server.c
$(SERVER): server.o
	$(CC) $(CFLAGS) -o $(SERVER) server.o

# Compile client object file
client.o: client.c
	$(CC) $(CFLAGS) -c client.c

# Compile server object file
server.o: server.c
	$(CC) $(CFLAGS) -c server.c -lssl -lcrypto

# Clean only object files after compilation
clean_o:
	rm -f *.o

# Clean all: object files and executables
clean:
	rm -f *.o $(CLIENT) $(SERVER)
