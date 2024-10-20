# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Executable names
CLIENT = client
SERVER = server

# Build the client and server programs
all: $(CLIENT) $(SERVER)

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
	$(CC) $(CFLAGS) -c server.c

# Clean up object files and executables
clean:
	rm -f *.o $(CLIENT) $(SERVER)
