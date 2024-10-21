# UFmyMusic

UFmyMusic is a networked application that synchronizes music libraries across multiple machines. Clients can interact with the server to list available files, compare their local files with those on the server, pull files from the server, and end their sessions.

## Compilation

To compile the project, navigate to the project directory and run the following command in your terminal:

```bash
make
```

## Running the Server
To start the server, use the following command:
```bash
./server
```
The server will begin listening for incoming client connections.

## Running the Client
To start the client, use the following command in a separate terminal window:
```bash
./client
```
The client will connect to the server and allow you to enter commands.

## Interface Functionality
In the client side, you will be prompted to enter one of the following commands:
**LIST**  Request a list of files currently stored on the server.
**DIFF**  Show the difference between files on the client and those on the server.
**PULL** Request to pull files from the server that are missing on the client.
**LEAVE** End the session with the server.
