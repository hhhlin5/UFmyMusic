/* Included libraries */
#include <stdio.h>		        /* for printf() and fprintf() */
#include <sys/socket.h>		    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Constants */
#define PORT 8080           
#define BUFFER_SIZE 1024	    /* The send buffer size */

/* Functions */
void showMenu(){
    printf("\n\n--- UFmyMusic Menu ---\n");
    printf("1. List Files\n");
    printf("2. Diff Files\n");
    printf("3. Pull File\n");
    printf("4. Leave\n");
    printf("Enter your choice: ");
}
// 1. List Files: Ask the server to return a list of files it currently has
void listFiles(int socket){
    char buffer[BUFFER_SIZE];
    send(socket, "LIST", 4, 0);
    recv(socket, buffer, BUFFER_SIZE, 0); 
    printf("\nA List of Files that Server has:\n%s", buffer);
}
// 2. Diff (based on 1)): the client should show a "diff" of the files it has in comparison to the server);
void diffFiles(int socket){
    char buffer[BUFFER_SIZE];
    send(socket, "DIFF", 4, 0);

    memset(buffer, 0, BUFFER_SIZE); 
    recv(socket, buffer, BUFFER_SIZE, 0); 
    printf("\nFiles not on client:\n%s", buffer);
}
// 3. Pull (request all files identified in 2): from the server and store them locally)
void pullFile(int socket){

}
// 4. Leave (the client should end its session with the server and take care of any open connections)
void leave(int socket){
    send(socket, "LEAVE", 5, 0);
    close(socket); 
}

/* The main function */
int main(){
    int clientSock;		            /* socket descriptor */
    struct sockaddr_in serv_addr;   /* The server address */

    /* Create a new TCP socket*/
    if ((clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        printf("socket() failed\n");
        exit(1);
    }

    /* Construct the server address structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(INADDR_ANY);
    serv_addr.sin_port = htons(PORT); 

    /* Establish connecction to the server */
    if (connect(clientSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("Connection failed\n"); 
        exit(1);
    }
    printf("Connected to the server.\n");

    while (1) {
        showMenu();
        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                listFiles(clientSock);
                break;
            case 2:
                diffFiles(clientSock);
                break;
            case 3:
                pullFile(clientSock);
                break;
            case 4:
                leave(clientSock);
                return 0; 
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
    return 0;
}