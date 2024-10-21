/* Included libraries */
#include <stdio.h>			/* For printf() and fprintf() */
#include <sys/socket.h>		/* For socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		/* For sockaddr_in and inet_addr() */
#include <stdlib.h>			/* Supports all sorts of functionality */
#include <unistd.h>			/* For close() */
#include <string.h>			/* Support any string ops */

/* Constants */
#define PORT 8888		   
#define BUFFER_SIZE 1024		/* The send buffer size */

/* Functions */
void showMenu();
void list_files(int clientSock);
void diff_files(int clientSock);
void pull_files(int clientSock);

/* The main function */
int main() {
	int clientSock;					/* socket descriptor */
	struct sockaddr_in serv_addr;   /* The server address */

	/* Create a new TCP socket*/
	if ((clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("socket() failed\n");
		return 1;
	}

	/* Construct the server address structure */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(PORT); 

	/* Establish connecction to the server */
	if (connect(clientSock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("Connection failed\n"); 
		return 1;
	}
	printf("Connected to the server.\n");

	while (1) {
		showMenu();
		int choice;
		scanf("%d", &choice);

		switch (choice) {
			case 1:
				list_files(clientSock);
				break;
			case 2:
				diff_files(clientSock);
				break;
			case 3:
				pull_files(clientSock);
				break;
			case 4:
				send(clientSock, "LEAVE", 5, 0);
				close(clientSock); 
				return 0;
			default:
				printf("Invalid choice. Try again.\n");
		}
	}
	return 0;
}

void showMenu() {
	printf("--- UFmyMusic Menu ---\n");
	printf("1. List Files\n");
	printf("2. Diff Files\n");
	printf("3. Pull File\n");
	printf("4. Leave\n");
	printf("Enter your choice: ");
}

// 1. List Files: Ask the server to return a list of files it currently has
void list_files(int clientSock) {
	char buffer[BUFFER_SIZE];
	send(clientSock, "LIST", 4, 0);
	recv(clientSock, buffer, BUFFER_SIZE, 0); 
	printf("\nA List of Files that Server has:\n%s\n", buffer);
}

// 2. Diff (based on 1)): the client should show a "diff" of the files it has in comparison to the server);
void diff_files(int clientSock) {
	char buffer[BUFFER_SIZE];
	send(clientSock, "DIFF", 4, 0);
	memset(buffer, 0, BUFFER_SIZE); 
	recv(clientSock, buffer, BUFFER_SIZE, 0); 
	printf("\nFiles not on client:\n%s\n", buffer);
}

// 3. Pull (request all files identified in 2): from the server and store them locally)
void pull_files(int clientSock) {

}
