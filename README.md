# UFmyMusic

UFmyMusic is a networked application that synchronizes music libraries across multiple machines. Clients can interact with the server to list available files, compare their local files with those on the server, pull files from the server, and end their sessions.

## Compilation

1. Ensure you have gcc complier installed.

2. Verify that you have all required source files (client.c, server.c, and makefile) and the directory files (music_server and music_client) where the server and client will manage in the project directory.

3. To compile the project, navigate to the project directory and run the following command in your terminal:

```bash
make
```

## Running the Server
To start the server, use the following command:
```bash
./server
```
### Server Functionality
1. The server will begin listening for incoming client connections after you run the command above. 

2. The server can multithreaded to handle multiple concurrent client requests. 

3. The server can store/retrieve historical information about each client in ./log.txt.

## Running the Client
To start the client, use the following command in a separate terminal window:
```bash
./client
```
### Client Interface
The client will connect to the server and allow you to enter commands. The interface that shown for client will be:

--- UFmyMusic Menu ---
1. List Files
2. Diff Files
3. Pull File
4. Leave

The user can interact with the menu by entering the corresponding number of their desired action. Each option triggers specific communication between the client and the server to perform the required function.

## Interface Functionality
In the client side, you will be prompted to enter one of the following commands:

**LIST**  Request a list of files currently stored on the server.

**DIFF**  Show the difference between files on the client and those on the server.

**PULL** Request to pull files from the server that are missing on the client.

**LEAVE** End the session with the server.

